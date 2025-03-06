Import("env")
import os

env.Replace(
    UPLOADER="echo",
    UPLOADCMD="wokwi-cli --token YOUR_WOKWI_TOKEN --project-dir $PROJECT_DIR"
)
