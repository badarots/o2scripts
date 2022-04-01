# O2 scripts

## stability.ipynb

Install all dependencies with:

    pip install --user --upgrade -r requirements.txt

Three files are needed to run the notebook:

1. Calibration with increasing statistics
2. Calibration histograms with increasing statistics

The above can be generated executing the `statStability.C` macro.

3. Time dependent calibration

This can be generated runing the time dependent calibrator

    o2-tpc-track-reader --disable-mc | o2-tpc-miptrack-filter | o2-tpc-calibrator-dedx --file-dump --field 2

don't forget to set the correct magnetic field, or leave it empty with you have the appropriate GRP file.

Or you can download the files a specific run: [run505658](https://cernbox.cern.ch/index.php/s/JpiMBNVg5HUW0G5).
