from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import pickle
import numpy as np

# Load trained models
with open("rf_glucose_model.pkl", "rb") as f:
    regressor, classifier = pickle.load(f)

app = FastAPI()

# Allow CORS for frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class InputData(BaseModel):
    HeartRate: float
    SpO2: float
    GSR: float

@app.post("/predict")
def predict_glucose(data: InputData):
    features = np.array([[data.HeartRate, data.SpO2, data.GSR]])
    glucose_pred = regressor.predict(features)[0]
    diabetes_status = classifier.predict(features)[0]
    return {
        "Predicted Glucose (mg/dL)": round(float(glucose_pred), 2),
        "Diabetes Status": diabetes_status
    }
