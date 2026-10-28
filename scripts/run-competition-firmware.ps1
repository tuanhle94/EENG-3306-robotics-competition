param([string]$Port = '', [switch]$MonitorOnly, [switch]$Pololu)

# Builds and uploads the competition program, then opens the serial monitor.
# -MonitorOnly skips the upload (read the robot's record of its last run).
# -Pololu is accepted so older instructions still work; it is the only build.

$ErrorActionPreference = 'Stop'
$workspacePath = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$localDependencies = Join-Path $workspacePath '.test-deps'
$pythonPath = 'C:\Users\Hang Minh Nguyen\AppData\Local\Programs\Python\Python312\python.exe'

Push-Location $workspacePath
try {
    $pio = Get-Command pio -ErrorAction SilentlyContinue
    if (-not $pio) {
        if (-not (Test-Path -LiteralPath $localDependencies)) { throw 'PlatformIO is not installed.' }
        $env:PYTHONPATH = $localDependencies
    }
    $arguments = @('run', '-e', 'uno_pololu_maze', '-t', 'upload')
    if ($Port) { $arguments += @('--upload-port', $Port) }
    if ($MonitorOnly) {
        Write-Output 'Monitor only: keeping the firmware already on the robot.'
    } else {
        Write-Output 'Building and uploading the competition program...'
        if ($pio) { & $pio.Source @arguments } else { & $pythonPath -m platformio @arguments }
        if ($LASTEXITCODE -ne 0) { throw "Upload failed with exit code $LASTEXITCODE." }
    }
    # log2file saves everything the robot prints, so a run is not lost if the
    # terminal stops displaying output. PlatformIO writes to .\logs itself.
    $logDirectory = Join-Path $workspacePath 'logs'
    $monitor = @('device', 'monitor', '--baud', '115200', '--filter', 'default', '--filter', 'log2file')
    if ($Port) { $monitor += @('--port', $Port) }
    Write-Output "Opening Serial Monitor. Every run is also saved in $logDirectory"
    if ($pio) { & $pio.Source @monitor } else { & $pythonPath -m platformio @monitor }
}
finally { Pop-Location }
