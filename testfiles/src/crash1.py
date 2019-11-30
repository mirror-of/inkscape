import os.path
import subprocess
import sys

inkscape_dir = sys.argv[1]
script_dir = os.path.dirname(os.path.abspath(__file__))
if subprocess.run([
        os.path.join(inkscape_dir, "bin", "inkscape"),
        "--convert-dpi-method=scale-viewbox",
        "--batch-process",
        os.path.join(script_dir, "GNU-Guile-logo.svg")
        ]).returncode != 0:
    sys.exit(1)
sys.exit(0)
