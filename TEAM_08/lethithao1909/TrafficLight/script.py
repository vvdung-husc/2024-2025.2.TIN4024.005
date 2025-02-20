from ast import Import
import venv


Import("env")
venv.Replace(
    UPLOADER="echo",
    UPLOADCMD="echo *** Use Wokwi to run this project! ***"
)
