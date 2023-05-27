$ErrorActionPreference = "Stop"

# Function to handle errors
function OnError {
    if ($LASTEXITCODE -eq 0) {
        Remove-Item -Path "running.log" -ErrorAction SilentlyContinue
        Write-Host "Done!!"
    }
    else {
        Write-Host "Error running with tigerbeetle"
        Get-Content "running.log"
    }

    Stop-Process -Id $args[0]
}
Register-EngineEvent PowerShell.Exiting -Action { OnError $PID }

# Be careful to use a running-specific filename so that we don't erase a real data file
$File = "$PWD/0_0.tigerbeetle"
if (Test-Path $File) {
    Remove-Item -Path $File -ErrorAction SilentlyContinue
}

Start-Process -FilePath "./zig-out/bin/tigerbeetle.exe" -ArgumentList "format", "--cluster=0", "--replica=0", "--replica-count=1", "$File" -RedirectStandardOutput "running.log" -RedirectStandardError "running.log" -NoNewWindow -PassThru | Out-Null
Write-Host "Starting replica 0"
Start-Process -FilePath "./zig-out/bin/tigerbeetle.exe" -ArgumentList "start", "--addresses=3001", "$File" -RedirectStandardOutput "running.log" -RedirectStandardError "running.log" -NoNewWindow -PassThru | Out-Null

Write-Host ""
Write-Host "running client..."
Start-Process -FilePath "../../tb_cpp.exe" -NoNewWindow -Wait

if (Test-Path $File) {
    Remove-Item -Path $File -ErrorAction SilentlyContinue
}
