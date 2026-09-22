# Brings the running VJ Engine window to the front, or launches a fresh
# instance if none is running. Never uses Start-Process/open_application-style
# "just launch it" logic without checking first - a second launch while one is
# already running creates a silent duplicate instance fighting over the same
# OSC port 9000 (see project notes: the "open_application always launches a
# new copy" gotcha). This script is the single safe entry point for both
# a human (Start VJ Session.bat) and the M4L device's "Show Display" button.
#
# Foregrounding note: raw SetForegroundWindow silently fails when the caller
# (powershell, spawned by Max, spawned by Live) isn't the OS's "last input"
# process - a well-known Windows restriction, not a bug in our P/Invoke.
# WScript.Shell's AppActivate goes through a different code path that isn't
# subject to that restriction and is the standard workaround; SetForegroundWindow
# is kept as a secondary attempt afterward, since AppActivate can occasionally
# leave the window merely flashing in the taskbar instead of fully focused.

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32ShowWindow {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr hWnd);
}
"@

function Bring-ToFront([System.Diagnostics.Process]$proc) {
    if ($proc.MainWindowHandle -eq [IntPtr]::Zero) { return $false }
    if ([Win32ShowWindow]::IsIconic($proc.MainWindowHandle)) {
        [Win32ShowWindow]::ShowWindow($proc.MainWindowHandle, 9) | Out-Null  # SW_RESTORE
    }
    try {
        $wshell = New-Object -ComObject wscript.shell
        $wshell.AppActivate($proc.Id) | Out-Null
    } catch {}
    [Win32ShowWindow]::SetForegroundWindow($proc.MainWindowHandle) | Out-Null
    return $true
}

$existing = Get-Process -Name "VJ Engine" -ErrorAction SilentlyContinue | Select-Object -First 1

if ($existing -and $existing.MainWindowHandle -ne [IntPtr]::Zero) {
    Bring-ToFront $existing | Out-Null
    Write-Output "brought existing VJ Engine (PID $($existing.Id)) to front"
} else {
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $release = Join-Path $scriptDir "build\VJEngine_artefacts\Release\VJ Engine.exe"
    $debug   = Join-Path $scriptDir "build\VJEngine_artefacts\Debug\VJ Engine.exe"
    $exe = if (Test-Path $release) { $release } else { $debug }
    $started = Start-Process -FilePath $exe -PassThru

    # Newly-created windows aren't always mapped yet, and can suffer the same
    # foreground restriction as an existing one - poll briefly for the window
    # handle to appear, then foreground it the same way.
    $deadline = (Get-Date).AddSeconds(5)
    $proc = $null
    while ((Get-Date) -lt $deadline) {
        $proc = Get-Process -Id $started.Id -ErrorAction SilentlyContinue
        if ($proc -and $proc.MainWindowHandle -ne [IntPtr]::Zero) { break }
        Start-Sleep -Milliseconds 200
        $proc = $null
    }
    if ($proc) {
        Bring-ToFront $proc | Out-Null
        Write-Output "launched fresh VJ Engine (PID $($started.Id)) from $exe and brought it to front"
    } else {
        Write-Output "launched fresh VJ Engine (PID $($started.Id)) from $exe - window not mapped yet after 5s"
    }
}
