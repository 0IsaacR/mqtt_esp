
# //-------------------------
# // Title: mqtt_node
# //-------------------------
# // Program Details:
# //-------------------------
# // Purpose: This program relays any mqtt traffic to a hostinger database. 

# // Inputs:  mqtt subscription
# // Outputs: database connection to hostinger
# // Date:  12/4/25
# // Author:  Isaac Rodriguez
# // Versions:
# //            V1 - passes voltage data to database
# //-------------------------
# // File Dependancies:
    # import time
    # import mysql.connector
    # import paho.mqtt.client as paho
    # from paho import mqtt
# //-------------------------
# // Main Program
# //-------------------------

import time
import mysql.connector
import paho.mqtt.client as paho
from paho import mqtt
# Database connection details / this section is similar to previous assignments
HOST = ""  # Replace with your Hostinger database IP
DB_USER = ""    # Replace with your database username
DB_PASSWORD = "" # Replace with your database password
DATABASE = "" # Replace with your database name


MQTT_USERNAME = ""
MQTT_PASSWORD = ""
clusterUrl = ""

sub_voltage = "testtopic/temp/outTopic/voltage"
sub_button = "testtopic/temp/outTopic/button"
sub_LED = "testtopic/temp/inTopic"

def push_value_to_db(sensor_value):
   try:
       # Connect to the database
       connection = mysql.connector.connect(
           host=HOST,
           user=DB_USER,
           password=DB_PASSWORD,
           database=DATABASE
       )

       if connection.is_connected():
           print("Connected to the database!")

           # Create a cursor object
           cursor = connection.cursor()

           # SQL query to insert data
           insert_query = "INSERT INTO mqtt_data (voltage) VALUES (%s)"
           cursor.execute(insert_query, (sensor_value,))

           # Commit the transaction
           connection.commit()
           print(f"Value {sensor_value} inserted into mqtt table!")

   except mysql.connector.Error as err:
       print(f"Error: {err}")
   finally:
        if connection.is_connected():
           cursor.close()
           connection.close()
           print("Database connection closed.")

# setting callbacks for different events to see if it works, print the message etc.
def on_connect(client, userdata, flags, rc, properties=None):
    print("CONNACK received with code %s." % rc)

# with this callback you can see if your publish was successful
def on_publish(client, userdata, mid, properties=None):
    print("mid: " + str(mid))

# print which topic was subscribed to
def on_subscribe(client, userdata, mid, granted_qos, properties=None):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

# print message, useful for checking if it was successful
def on_message(client, userdata, msg):
    # Example: Push a sensor value
    sensor_value = msg.payload 
    push_value_to_db(sensor_value)
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))

# using MQTT version 5 here, for 3.1.1: MQTTv311, 3.1: MQTTv31
# userdata is user defined data of any type, updated by user_data_set()
# client_id is the given name of the client
client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect

# enable TLS for secure connection
client.tls_set(tls_version=mqtt.client.ssl.PROTOCOL_TLS)
# set username and password
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
# connect to HiveMQ Cloud on port 8883 (default for MQTT)
client.connect(clusterUrl, 8883)

# setting callbacks, use separate functions like above for better visibility
client.on_subscribe = on_subscribe
client.on_message = on_message
client.on_publish = on_publish

# subscribe to all topics of encyclopedia by using the wildcard "#"
client.subscribe(sub_voltage, qos=0)

# a single publish, this can also be done in loops, etc.
# client.publish("encyclopedia/temperature", payload="hot", qos=1)

# loop_forever for simplicity, here you need to stop the loop manually
# you can also use loop_start and loop_stop
client.loop_forever()
