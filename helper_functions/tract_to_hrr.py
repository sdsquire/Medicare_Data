import pandas as pd
import os.path

pwd = os.path.dirname(__file__)

zip_tract = pd.read_csv(pwd+"/../tract_to_zip.csv", dtype={"zip":"string", "tract":"string"})
#zip_tract = pd.read_csv(pwd+"/../raw_data/Crosswalk/tract_zip.csv", dtype={"zip":"string", "tract":"string"})
zip_hrr = pd.read_csv(pwd+"/../raw_data/Crosswalk/zip_hrr.csv", dtype={"zip":"string", "HRR":"string"})
tract_hrr = zip_tract.merge(zip_hrr)
tract_hrr.to_csv(pwd + "/../processed_data/tract_to_hrr.csv", index=False)
