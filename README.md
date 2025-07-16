# Real-Time-Stress-Level-Detection-System-Using-IoT-Sensors-and-Machine-Learning-

1. Project Idea
   
This project aims to develop a real-time stress detection system using physiological data 
collected from human subjects via sensors connected to an ESP32 microcontroller. The goal is 
to classify a person's stress level into three categories — "Relaxed", "Stressed", and 
"Highly Stressed" — and trigger appropriate actuator responses via MQTT communication 
based on the prediction. The final system is capable of receiving live sensor data, performing 
machine learning-based classification, and activating visual alerts (LEDs) corresponding to 
the detected stress level. 

2. Methodology
   
The system architecture consists of the following major components: 
• Sensor Data Acquisition 
In this project, three physiological sensors were connected to an ESP32 microcontroller to 
collect biometric data from human subjects: 
o DHT11 Sensor – Used to measure body temperature and surrounding humidity. 
o GSR (Galvanic Skin Response) Sensor – Captures changes in the skin's electrical 
conductance, which vary with emotional arousal and stress levels. 
o Heart Pulse Sensor – Monitors the heart rate in beats per minute , which is a key 
physiological indicator of stress. 
For each participant, a continuous 1-minute session of sensor data was recorded under 
controlled conditions. During this period, the subject maintained a static posture to minimize 
motion-related noise, and the ESP32 collected synchronized readings from all three sensors at 
regular intervals. 

• Data Labeling and Dataset Creation: 

The collected physiological data were manually annotated based on the participant’s known 
emotional state during the recording session. Emotional states were either self-reported by the 
subject or induced in a controlled environment (e.g., calm background for relaxed state, 
mental arithmetic for stressed state). 
Each 1-minute data segment collected from a subject was labeled as one of the 
following: 
• Relaxed 
• Stressed 
• Highly Stressed 

These labeled segments formed the basis of the dataset. Each entry in the dataset 
represents a time-windowed snapshot of temperature, humidity, skin conductance 
(GSR), and heart rate, along with the corresponding stress level label. 
To ensure data quality and reduce noise, preprocessing techniques such as 
normalization were applied prior to training the machine learning model.We share the 
entire dataset (csv file) in the attachment.( Dataset has been increased.) 

• Model Training and Evaluation: 

After the dataset was preprocessed and grouped, multiple classification algorithms were 
evaluated to determine the most accurate and generalizable model. The primary goal was to 
accurately classify physiological signals into one of the three stress states: Relaxed, Stressed, 
and Highly Stressed. 

Modeling Approach 

The dataset was split into features X = [temperature, humidity, gsr, pulse] and target y = label. 
Labels were encoded using LabelEncoder to convert string classes into numeric values. 
Stratified splitting ensured that all classes were proportionally represented in both training and 
test sets. 
Several machine learning algorithms were experimented with, including: 
• Decision Tree Classifier 
• K-Nearest Neighbors 
• Support Vector Machine 
• Random Forest Classifier 

Among these, the Random Forest Classifier provided the most stable and accurate results. 
The selected model was fine-tuned using the following hyperparameters: 

Feature Engineering Attempts 

In order to explore whether model performance could be improved, several additional feature 
engineering approaches were tested: 
• Feature scaling and normalization 
• Statistical features like standard deviation, median, and variance per session 
• Derived metrics such as pulse-to-GSR ratios and humidity-temperature differentials 
However, these additions did not improve the model's performance significantly. In some 
cases, they led to overfitting or added noise. Through repeated experimentation, it was 
concluded that the mean values of the four primary features offered the most robust and 
interpretable results. Thus, the final model was trained using only those core features.

• Model Deployment:   

After successfully training and evaluating the Random Forest Classifier, the model was 
prepared for real-time deployment to classify incoming physiological data streamed from an 
ESP32 microcontroller via MQTT. 

Model and Label Encoder Serialization 

To ensure reusability and avoid retraining the model each time, the trained Random Forest 
model and the corresponding LabelEncoder were serialized using the joblib library: 
joblib.dump(model, "stress_rf_model.pkl") 
joblib.dump(le, "label_encoder.pkl") 
These files (stress_rf_model.pkl and label_encoder.pkl) contain the complete structure and 
learned parameters of the model, enabling them to be loaded instantly in a production 
environment. 
• MQTT Communication Workflow: 
o The ESP32 published real-time sensor data to a specific MQTT topic. 
o A Local PC Python client subscribed to this topic, received the data, and 
passed it to the ML model. 
o The prediction result (label) was then published to another MQTT topic. 
o The ESP32, subscribed to the output topic, triggered a corresponding LED 
color: 
▪ Green for "Relaxed" 
▪ Blue for "Stressed" 
▪ Red for "Highly Stressed"

System Integration Overview 

• Edge Device: Handles model inference (local PC) 
• ESP32: Collects and sends sensor data; receives prediction and activates actuators 
• MQTT Broker: Enables communication between ESP32 and inference system 
• Pre-trained ML Model: Efficiently classifies emotional state 
This architecture allows for low-latency, scalable, and interpretable deployment of the stress 
classification model in real-world biofeedback or wellness monitoring applications. 

3. Dataset
   
• Total Samples: 4020 row 
• Features Collected: 
o Temperature (°C) 
o Humidity (%) 
o GSR Value (Ω or normalized) 
o Heart Rate  
• Labels: 
o Relaxed (0) 
o Stressed (1) 
o Highly Stressed (2) 
• Preprocessing: 
o Missing values were removed. 
o Features were normalized using StandardScaler. 
o The dataset was split into 80% training and 20% testing. 

4. Machine Learning Model
   
Selected Algorithm: Random Forest Classifier 
Tools Used: Python, Scikit-learn 
Dataset Size: 60-second averaged feature vectors from multiple subjects 
Number of Features: 4 (temperature, humidity, GSR, pulse) 

Feature Engineering Observations: 

Various feature engineering strategies were tested — including statistical metrics (e.g., 
standard deviation, variance), derived ratios (e.g., GSR/pulse), and feature scaling — but no 
significant improvement in accuracy was observed. The most optimal results were obtained 
using mean values of each 60-second sensor window. 
The model showed robust generalization ability. Thanks to the use of 
class_weight='balanced' and careful parameter tuning, it successfully avoided overfitting. 
Consistent performance was observed across different random test splits and class 
distributions. 

5. Results
   
• Real-time Prediction: 
The model was successfully integrated into the live system. Predictions were made 
within milliseconds after receiving sensor data via MQTT. 
• Output Control: 
Based on the predicted label: 

o Green LED: Turned on when the user is relaxed 
o Blue LED: Turned on when the user is under moderate stress 
o Red LED: Turned on when the user is highly stressed 

• System Responsiveness: 
The total response time from sensor data acquisition to LED activation was typically 
below 2 seconds, enabling a near real-time interaction. 

• Deployment Success: 
The project was tested in real environments and successfully responded to 
physiological changes in the subject, validating both the model and the embedded 
system. 

6. Conclusion
   
This project demonstrates the effective integration of IoT and machine learning for real-time 
human state detection. Through the use of low-cost sensors, an ESP32 microcontroller, and a 
reliable MQTT communication framework, we were able to classify stress levels with high 
accuracy and trigger physical outputs accordingly. The Random Forest model provided 
excellent performance, making it suitable for real-time deployment.
