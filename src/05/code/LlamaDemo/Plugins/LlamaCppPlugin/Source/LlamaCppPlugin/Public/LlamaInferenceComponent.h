#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HAL/Runnable.h"
#include "LlamaInferenceComponent.generated.h"

// Forward declarations
struct llama_model;
struct llama_context;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseReceived, const FString&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenGenerated, const FString&, Token);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LLAMACPPPLUGIN_API ULlamaInferenceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULlamaInferenceComponent();
    virtual ~ULlamaInferenceComponent();

    UFUNCTION(BlueprintCallable, Category = "Llama")
    bool LoadModel(const FString& ModelPath, int32 ContextSize = 2048);
    
    UFUNCTION(BlueprintCallable, Category = "Llama")
    void GenerateResponse(const FString& Prompt);
    
    UFUNCTION(BlueprintCallable, Category = "Llama")
    void UnloadModel();
    
    UFUNCTION(BlueprintCallable, Category = "Llama")
    bool IsModelLoaded() const { return bModelLoaded; }

    UPROPERTY(BlueprintAssignable, Category = "Llama")
    FOnResponseReceived OnResponseReceived;
    
    UPROPERTY(BlueprintAssignable, Category = "Llama")
    FOnTokenGenerated OnTokenGenerated;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    llama_model* Model;
    llama_context* Context;
    bool bModelLoaded;
    
    class FLlamaInferenceTask* CurrentTask;
    
    // Make these public so the task can call them
    friend class FLlamaInferenceTask;
    void OnInferenceComplete(const FString& Response);
    void OnTokenStreamed(const FString& Token);
};

// Async task for running inference on background thread
class FLlamaInferenceTask : public FRunnable
{
public:
    FLlamaInferenceTask(llama_context* InContext, llama_model* InModel,
                        const FString& InPrompt, ULlamaInferenceComponent* InComponent);
    virtual ~FLlamaInferenceTask();
    
    virtual uint32 Run() override;
    virtual void Stop() override;
    
private:
    llama_context* Context;
    llama_model* Model;
    FString Prompt;
    ULlamaInferenceComponent* Component;
    FRunnableThread* Thread;
    bool bStopRequested;
};