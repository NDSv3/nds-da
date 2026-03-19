# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'NDS-DeviceAccess'
copyright = '2026, David Andrino - I2A2'
author = 'David Andrino - I2A2'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [ "breathe", "exhale", "sphinxcontrib.mermaid" ]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']
html_extra_path = ['_coverage']

breathe_projects = {
    "NDS-DeviceAccess": "_doxygen/xml"
}
breathe_default_project = "NDS-DeviceAccess"

exhale_args = {
    "containmentFolder":     "./_exhale",
    "rootFileName":          "ndsda-api.rst",
    "rootFileTitle":         "API Reference",
    "doxygenStripFromPath":  "../src/main/c++/include",

    "createTreeView":        False,
    "exhaleExecutesDoxygen": True,
    "contentsDirectives": False,
    "exhaleDoxygenStdin":    """
        INPUT      = ../src/main/c++/include
        RECURSIVE  = YES
    """,
}