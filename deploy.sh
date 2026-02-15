#!/bin/bash

# Deploy to Google Cloud Run
# Requires gcloud CLI to be installed and authenticated

echo "Deploying to Google Cloud Run..."
gcloud run deploy rox-playground \
    --source . \
    --platform managed \
    --region us-east1 \
    --port 3000

echo "Done!"
