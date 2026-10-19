# PowerShell build and verification script for OpenMP Tutorial 05
# Explicitly uses MSYS2 UCRT64 GCC and Python 3.12

$GCC = "C:\msys64\ucrt64\bin\gcc.exe"
$PYTHON = "C:\Users\admin h\AppData\Local\Programs\Python\Python312\python.exe"

# Prepend MSYS2 UCRT64 bin directory to ensure runtime DLLs (libgomp-1.dll, etc.) are available
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH

if (-not (Test-Path $GCC)) {
    Write-Error "GCC compiler not found at $GCC. Please check your MSYS2 UCRT64 installation."
    exit 1
}

Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host " Building and Benchmarking OpenMP Tutorial 05 Programs" -ForegroundColor Cyan
Write-Host " Compiler: $(& $GCC --version | Select-Object -First 1)" -ForegroundColor Cyan
Write-Host "====================================================================" -ForegroundColor Cyan

New-Item -ItemType Directory -Force -Path "bin", "results", "graphs", "screenshots" | Out-Null

$sources = @(
    @{ 
        Name = "Question 1: Race Conditions & Synchronization"; 
        Src = "src/q1_race_condition.c"; 
        Out = "bin/q1_race_condition.exe"; 
        Res = "results/q1_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra" 
    },
    @{ 
        Name = "Question 2: Scheduling, Barriers, and Nowait"; 
        Src = "src/q2_scheduling_nowait.c"; 
        Out = "bin/q2_scheduling_nowait.exe"; 
        Res = "results/q2_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra" 
    },
    @{ 
        Name = "Question 3: Monte Carlo Pi Estimation"; 
        Src = "src/q3_monte_carlo_pi.c"; 
        Out = "bin/q3_monte_carlo_pi.exe"; 
        Res = "results/q3_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra" 
    },
    @{ 
        Name = "Question 4: Parallel Binary Tree Sum"; 
        Src = "src/q4_binary_tree_sum.c"; 
        Out = "bin/q4_binary_tree_sum.exe"; 
        Res = "results/q4_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra" 
    },
    @{ 
        Name = "Question 5: Parallel BFS & DFS Traversals"; 
        Src = "src/q5_bfs_dfs.c"; 
        Out = "bin/q5_bfs_dfs.exe"; 
        Res = "results/q5_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra -Wl,--stack,67108864" 
    },
    @{ 
        Name = "Question 6: Default Scheduling Policy Detection"; 
        Src = "src/q6_default_schedule.c"; 
        Out = "bin/q6_default_schedule.exe"; 
        Res = "results/q6_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra" 
    },
    @{ 
        Name = "Optional: Steady-State Heat Conduction"; 
        Src = "src/optional_heat_conduction.c"; 
        Out = "bin/optional_heat_conduction.exe"; 
        Res = "results/optional_heat_output.txt"; 
        Flags = "-O3 -fopenmp -Wall -Wextra" 
    }
)

foreach ($item in $sources) {
    Write-Host "`n--> Compiling $($item.Name)..." -ForegroundColor Yellow
    $flags = $item.Flags -split ' '
    & $GCC $flags $item.Src -o $item.Out -lm
    if ($LASTEXITCODE -ne 0) {
        Write-Host "    [FAILED] Compilation error in $($item.Src)" -ForegroundColor Red
        continue
    }
    Write-Host "    [OK] Compiled successfully to $($item.Out)." -ForegroundColor Green

    Write-Host "--> Executing $($item.Out)..." -ForegroundColor Yellow
    $output = & $item.Out
    $output | Out-File -FilePath $item.Res -Encoding utf8
    $output | ForEach-Object { Write-Host $_ }
    Write-Host "    [SAVED] Output recorded in $($item.Res)" -ForegroundColor Green
}

# 1. Generate graphs
Write-Host "`n--> Generating Monte Carlo Graphs via Python 3.12..." -ForegroundColor Yellow
& $PYTHON "plot_pi.py"

# 2. Render terminal screenshots
Write-Host "`n--> Generating Execution Screenshots..." -ForegroundColor Yellow
& $PYTHON "render_screenshots.py"

Write-Host "`n====================================================================" -ForegroundColor Cyan
Write-Host " OpenMP Tutorial 05 - Build and Verification Complete!" -ForegroundColor Cyan
Write-Host "====================================================================" -ForegroundColor Cyan
