import os.path
import pandas as pd
import requests
import re

pwd = os.path.dirname(__file__)
claims_data = pd.read_csv(pwd + "/../processed_data/Physician_Data.csv")

claims_data["Lat"] = round(2 * claims_data["Latitude"]) / 2
claims_data["Long"] = round(2 * claims_data["Longitude"]) / 2

count = 0
def get_fips12(latitude, longitude, index=None):
	if index is not None:
		print(f"Locating physician {index}")
	res = requests.get(f"https://geo.fcc.gov/api/census/block/find?latitude={latitude}&longitude={longitude}&censusYear=2020&format=xml")
	return re.match(r".*FIPS=\"(\d*)\" bbox.*", res.text).group(1)[:12]

claims_data["Rndrng_Prvdr_Fips"] = claims_data.apply(lambda row: get_fips12(row["Latitude"], row["Longitude"], row["Rndrng_NPI"]), axis = 1)

claims_data.to_csv(pwd + "/../processed_data/Physician_Data.csv")
