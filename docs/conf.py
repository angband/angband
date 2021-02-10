# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = "Angband"
copyright = "2019, Angband developers past and present"
author = "Angband developers past and present"

# The full version, including alpha/beta/rc tags
import subprocess
# Python 3.5 introduces subprocess.run(); use check_output() instead in case
# the system's Python is older than that.
version = subprocess.check_output(['../scripts/version.sh'],
        universal_newlines=True)
release = version

# -- General configuration ---------------------------------------------------

# 2.0 changed the default value to 'index'.  Set this manually for backwards
# compatibility with previous versions.
master_doc = 'index'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = []

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
from better import better_theme_path

html_theme_path = [better_theme_path]
html_theme = "better"

html_title = "<img> The Angband Manual"
html_short_title = "Home"
html_sidebars = {
    "**": ["localtoc.html", "searchbox.html"],
    "index": ["globaltoc.html", "searchbox.html"],
}
html_theme_options = {
    "cssfiles": ["_static/style.css"],
    "showheader": True,
    "textcolor": "rgb(230, 230, 242)",
    "headtextcolor": "rgb(253, 229, 164)",
}


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
