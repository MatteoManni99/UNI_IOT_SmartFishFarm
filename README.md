# Smart FishFarm

In this project, I developed a smart environment using IoT techniques. The use case is an aquaculture system. Naturally, many things were simplified. I worked with five sensors and used ContikiNG as the operating system for the latter. One of them is utilized as an RPL Border Router for the device network, while the other four devices simulate the following: temperature (sensor), water quality (sensor), heat pump (actuator), and water filter (actuator). The communication protocols involved are MQTT and CoAP. The project is essentially divided into two parts: the sensor network, developed in C, and the main application, developed in Java.

For more details, you can refer to the documentation.
