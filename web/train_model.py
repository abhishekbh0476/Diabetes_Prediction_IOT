import pandas as pd
from sklearn.ensemble import RandomForestRegressor, RandomForestClassifier
import pickle

# Load dataset
df = pd.read_csv("glucose_dataset.csv")

# Features & Target
X = df[["HeartRate", "SpO2", "GSR"]]
y_glucose = df["Glucose"]

# Create Classification Labels
df["Label"] = pd.cut(
    df["Glucose"],
    bins=[0, 110, 140, 1000],
    labels=["Non-Diabetic", "Pre-Diabetic", "Diabetic"]
)
y_status = df["Label"]

# Train regression model (for glucose prediction)
regressor = RandomForestRegressor(n_estimators=300, random_state=42)
regressor.fit(X, y_glucose)

# Train classification model (for diabetes prediction)
classifier = RandomForestClassifier(n_estimators=300, random_state=42)
classifier.fit(X, y_status)

# Save both models
with open("rf_glucose_model.pkl", "wb") as f:
    pickle.dump((regressor, classifier), f)

