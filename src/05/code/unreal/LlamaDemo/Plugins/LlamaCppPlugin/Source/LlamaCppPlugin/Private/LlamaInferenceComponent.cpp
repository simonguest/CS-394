#include "LlamaInferenceComponent.h"
#include "llama.h"
#include "HAL/RunnableThread.h"
#include "Async/Async.h"
#include <string>
#include <vector>

ULlamaInferenceComponent::ULlamaInferenceComponent()
    : Model(nullptr)
    , Context(nullptr)
    , bModelLoaded(false)
    , CurrentTask(nullptr)
{
    PrimaryComponentTick.bCanEverTick = false;
}

ULlamaInferenceComponent::~ULlamaInferenceComponent()
{
    UnloadModel();
}

void ULlamaInferenceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize llama backend
    llama_backend_init();
}

void ULlamaInferenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnloadModel();
    llama_backend_free();
    
    Super::EndPlay(EndPlayReason);
}

bool ULlamaInferenceComponent::LoadModel(const FString& ModelPath, int32 ContextSize)
{
    if (bModelLoaded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Model already loaded. Unload first."));
        return false;
    }
    
    // Convert FString to std::string
    std::string PathStr(TCHAR_TO_UTF8(*ModelPath));
    
    // Set up model parameters
    llama_model_params model_params = llama_model_default_params();
    
    // Load the model using new API
    Model = llama_model_load_from_file(PathStr.c_str(), model_params);
    if (!Model)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load model from: %s"), *ModelPath);
        return false;
    }
    
    // Create context using new API
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = ContextSize;
    ctx_params.n_threads = 4;
    
    Context = llama_init_from_model(Model, ctx_params);
    if (!Context)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create context"));
        llama_model_free(Model);
        Model = nullptr;
        return false;
    }
    
    bModelLoaded = true;
    UE_LOG(LogTemp, Log, TEXT("Model loaded successfully: %s"), *ModelPath);
    return true;
}

void ULlamaInferenceComponent::GenerateResponse(const FString& Prompt)
{
    if (!bModelLoaded)
    {
        UE_LOG(LogTemp, Error, TEXT("No model loaded"));
        return;
    }
    
    if (CurrentTask)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inference already in progress"));
        return;
    }
    
    // Start async inference
    CurrentTask = new FLlamaInferenceTask(Context, Model, Prompt, this);
}

void ULlamaInferenceComponent::UnloadModel()
{
    if (CurrentTask)
    {
        CurrentTask->Stop();
        delete CurrentTask;
        CurrentTask = nullptr;
    }
    
    if (Context)
    {
        llama_free(Context);
        Context = nullptr;
    }
    
    if (Model)
    {
        llama_model_free(Model);
        Model = nullptr;
    }
    
    bModelLoaded = false;
}

void ULlamaInferenceComponent::OnInferenceComplete(const FString& Response)
{
    // Dispatch to game thread
    AsyncTask(ENamedThreads::GameThread, [this, Response]()
    {
        OnResponseReceived.Broadcast(Response);
        
        if (CurrentTask)
        {
            delete CurrentTask;
            CurrentTask = nullptr;
        }
    });
}

void ULlamaInferenceComponent::OnTokenStreamed(const FString& Token)
{
    // Dispatch to game thread
    AsyncTask(ENamedThreads::GameThread, [this, Token]()
    {
        OnTokenGenerated.Broadcast(Token);
    });
}

// ===== FLlamaInferenceTask Implementation =====

FLlamaInferenceTask::FLlamaInferenceTask(llama_context* InContext, llama_model* InModel,
                                         const FString& InPrompt, ULlamaInferenceComponent* InComponent)
    : Context(InContext)
    , Model(InModel)
    , Prompt(InPrompt)
    , Component(InComponent)
    , bStopRequested(false)
{
    Thread = FRunnableThread::Create(this, TEXT("LlamaInferenceThread"), 0, TPri_BelowNormal);
}

FLlamaInferenceTask::~FLlamaInferenceTask()
{
    if (Thread)
    {
        Thread->Kill(true);
        delete Thread;
    }
}

uint32 FLlamaInferenceTask::Run()
{
    // Get the vocab from model
    const llama_vocab* vocab = llama_model_get_vocab(Model);
    
    // Tokenize the prompt
    std::string PromptStr(TCHAR_TO_UTF8(*Prompt));
    std::vector<llama_token> tokens;
    
    // Get number of tokens needed
    const int n_tokens = -llama_tokenize(
        vocab,
        PromptStr.c_str(),
        PromptStr.length(),
        nullptr,
        0,
        true,
        true
    );
    
    tokens.resize(n_tokens);
    llama_tokenize(
        vocab,
        PromptStr.c_str(),
        PromptStr.length(),
        tokens.data(),
        tokens.size(),
        true,
        true
    );
    
    // Clear KV cache using the seq_id API
    // llama_kv_cache_seq_rm(Context);
    
    // Create batch
    llama_batch batch = llama_batch_init(512, 0, 1);
    
    // Manually add tokens to batch
    batch.n_tokens = 0;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        batch.token[batch.n_tokens] = tokens[i];
        batch.pos[batch.n_tokens] = i;
        batch.n_seq_id[batch.n_tokens] = 1;
        batch.seq_id[batch.n_tokens][0] = 0;
        batch.logits[batch.n_tokens] = false;
        batch.n_tokens++;
    }
    // Request logits for last token
    batch.logits[batch.n_tokens - 1] = true;
    
    // Decode the prompt
    if (llama_decode(Context, batch) != 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decode prompt"));
        llama_batch_free(batch);
        return 0;
    }
    
    // Generate tokens
    FString FullResponse;
    const int max_tokens = 512;
    
    for (int i = 0; i < max_tokens && !bStopRequested; i++)
    {
        auto* logits = llama_get_logits_ith(Context, batch.n_tokens - 1);
        auto n_vocab = llama_vocab_n_tokens(vocab);
        
        // Simple greedy sampling
        llama_token new_token = 0;
        float max_logit = logits[0];
        for (int j = 1; j < n_vocab; j++)
        {
            if (logits[j] > max_logit)
            {
                max_logit = logits[j];
                new_token = j;
            }
        }
        
        // Check for end of generation
        if (llama_vocab_is_eog(vocab, new_token))
        {
            break;
        }
        
        // Convert token to text
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token, buf, sizeof(buf), 0, true);
        if (n < 0)
        {
            break;
        }
        
        FString TokenText(UTF8_TO_TCHAR(std::string(buf, n).c_str()));
        FullResponse += TokenText;
        
        // Stream token to UI
        Component->OnTokenStreamed(TokenText);
        
        // Prepare next batch - manually set up
        batch.n_tokens = 0;
        batch.token[0] = new_token;
        batch.pos[0] = tokens.size() + i;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = true;
        batch.n_tokens = 1;
        
        if (llama_decode(Context, batch) != 0)
        {
            break;
        }
    }
    
    llama_batch_free(batch);
    
    // Send complete response
    Component->OnInferenceComplete(FullResponse);
    
    return 0;
}

void FLlamaInferenceTask::Stop()
{
    bStopRequested = true;
}