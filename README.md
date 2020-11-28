# HTAnalyzer
Handy Terminal Protocol Low Level Analyzer for saleae Logic2

## To build:
1. Clone and cd to this repo.
2. `git submodule init`
3. `git submodule update`
4. `./build.sh`

## To load to Saleae Logic2 (As of writing, Logic2.3.11 is used)
1. Obtain an HT digital sample
2. Go to `Preferences` from the hamburger menu at the bottom

3. Set the directory to `<path to repo>/debug`

4. Reload Logic2
5. Load the HT Analyzer like you would any Low Level Analyzer (e.g. Serial/CAN):