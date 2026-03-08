---
name: generate-resources
description: Generates a resources.qmd file from a slides.qmd file in the same module directory.
---

When generating a resources.qmd file:

1. If you are not given a slides.qmd file, prompt for one.
2. Create a new resources.qmd file in the same directory as the slides.qmd if doesn't exist. Append if it does exist.
3. Look through all of the URLs in the given slides.qmd file. Group related ones under sections.
4. Extract all of the URLs using the following format:

## Section Header

- [Short description](https://linktourl.com) - One sentence description of the resource

For example, two resources related to Hugging Face would be listed as follows:

## Hugging Face

- [Hugging Face Hub](https://huggingface.co) - Central repository for sharing models, datasets, and demos
- [Datasets Documentation](https://huggingface.co/docs/datasets) - Library for loading, processing, and publishing datasets