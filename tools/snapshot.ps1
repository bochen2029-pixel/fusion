# tools/snapshot.ps1 — DoD clause 6: full-tree snapshot (captures gitignored runs/)
# Usage: pwsh C:/fusion/tools/snapshot.ps1   (forward slashes in all invocations)
# <ts> format is pinned: yyyyMMdd-HHmmss (P-17).
$ErrorActionPreference = "Stop"
$src  = "C:/fusion"
$ts   = Get-Date -Format "yyyyMMdd-HHmmss"
$dest = "C:/fusion_snapshots/$ts"
New-Item -ItemType Directory -Force -Path $dest | Out-Null
# robocopy: mirror the tree; exclude build products and vendored downloads, KEEP runs/.
$excludeDirs = @("build", "build_Release", "out", ".vs", "node_modules", "third_party/dl", "Testing")
$xd = $excludeDirs | ForEach-Object { Join-Path $src $_ }
robocopy $src $dest /MIR /XD @xd /XF "*.obj" "*.pdb" /R:2 /W:2 /NFL /NDL /NP | Out-Null
if ($LASTEXITCODE -ge 8) { throw "robocopy failed with $LASTEXITCODE" }
Write-Host "snapshot -> $dest (robocopy code $LASTEXITCODE)"
