Import("env")

# Multiple actions
env.AddCustomTarget(
    name="upload_remote",
    dependencies=None,
    actions=[
        "C:\\Users\\Niko\\.platformio\\penv\\Scripts\\platformio.exe remote run --target upload --environment esp12e"
    ],
    title="Upload Remote",
    description="Builds the projects and uploads it to the remote device"
)

env.AddCustomTarget(
    name="monitor_remote",
    dependencies=None,
    actions=[
        "C:\\Users\\Niko\\.platformio\\penv\\Scripts\\platformio.exe remote device monitor"
    ],
    title="Monitor Remote",
    description="Opens a monitor on the remote device"
)