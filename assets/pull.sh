#!/bin/bash
# Get some datasets in XML for further usage in the and benchmarks.
# Some are pretty heavy, but it is needed for stress testing

wget "https://ec.europa.eu/eurostat/api/dissemination/sdmx/2.1/data/tipspd40?format=sdmx_2.1_structured&compressed=true" -O tipspd40.xml.gz
wget "https://ec.europa.eu/eurostat/api/dissemination/sdmx/2.1/data/nasa_10_f_bs?format=sdmx_2.1_structured&compressed=true" -O nasa_10_f_bs.xml.gz
wget "https://ec.europa.eu/eurostat/api/dissemination/sdmx/2.1/data/sdg_09_40?format=sdmx_2.1_structured&compressed=true" -O sdg_09_40.xml.gz

gzip -d *.gz