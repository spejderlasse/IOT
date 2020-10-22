# IOT
Repository for the course E5IOT at Aarhus University, 2020

This repository will contain sourcecode for the IOT project of Lasse Greve Rasmussen

The project will be developed for a particle Argon

The project is targeted a cicken coop, and the purpose is to set up a device to:


1. piority:

- collect daily sunrise/sunset from a webhook

- by a servo to close the door one hour after sunset

- by a servo to open the door at sunrise

- allow a user to manually open or close the door


2. priority

- powered by solar

- ligth for evenings/monings in vintertime

------------------------------------------------------------------------------------------------

Defined as requirements:

- The system must collect data of sunrise and sunset from an API on dayli basis
- The system must controle a servo to open and close the gate
- When the time of sunset is reached, the gate must close
- When the time of sunrise is reached, the gate must open
- While nothing to proceed the system should go to sleep
- If sunrise and sunset isn’t updated, the system will be controlled by lightlevels meassured by a photoresistor
- The system could be driven by a battery charged froma solacell
- While the nights are long in wintertime, there could be turned on ligth in the Henhouse for some houers after sunset
