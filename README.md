# IvyBells
# A C2 Framework for operating within Windows environments(alpha v0.1)
-Name is inspired by covert Operation Ivy Bells -- > https://en.wikipedia.org/wiki/Operation_Ivy_Bells


# TO DO
- Improve Operator client to make it more dynamic and have less one-off glitches
- Increase enumeration information sent to server when agents first register with the server
- Add reflective loading capabilities to implant DLL
- Add self-destruct capabilities to implant 
- Add migration capabilities to implant
- Add mechanism to obscure IOC DLLs in the PEB(libcurl.dll, netapi.dll)
- Reduce number of strings by filling out the dynamicresolution.h structures to be used throughout the code
- Add Download functionality
- Add the abilities to transfer files between the Operator Client and the server
- Add modules that add capabilities
- Make use of the Twilio API to integrate SMS capabilties with the server
- Improve security of the server by adding API keys for the Operator Client and a randomized auth mechanism for the agents
- Integrate a certificate for the server to be able to run HTTPS comms
- Introduce an easy to modify config.h file that will be used for configuring agents before compilation
  - Need dll spoof list and server config here for sure
- add pivoting capabilities

# Server Features
- highly dynamic and scalable REST API
- Serves numerous payloads
- Login/Authentication
- Easy to deploy and built for Docker
- Controlled completely via Operator CLI client

# Implant Features
- Retrieve various payloads and load them reflectively
- Obfuscation scheme that will hide and obscure strings and imports
- View and kill processes, enumerate users, exfiltrate files, navigate the file system
- Ability to integrate custom modules/DLLs at will that add commands and extend functionality
- Hides reflectively loaded DLLs by loading a benign DLL, parsing the PEB, and changing where it points

# Operator CLI client Features
- Ability to register new modules and dynamically add features to implants
- Communicate with server from anywhere
- Highly dynamic and intuitive interface
- One stop shop for command and control of agents
