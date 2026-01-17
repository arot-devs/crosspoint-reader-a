param(
  [Parameter(Mandatory = $true)]
  [string]$InputHtml,

  [Parameter(Mandatory = $true)]
  [string]$OutputEpub,

  [string]$Title,
  [int]$SplitLevel = 2,
  [int]$TocDepth = 3,
  [string]$OutputHtml
)

$ErrorActionPreference = 'Stop'

$argsList = @(
  "scripts/html_to_epub_mathfix.py",
  $InputHtml,
  $OutputEpub,
  "--split-level=$SplitLevel",
  "--toc-depth=$TocDepth"
)

if ($Title) {
  $argsList += "--title=$Title"
}

if ($OutputHtml) {
  $argsList += "--output-html=$OutputHtml"
}

python @argsList
