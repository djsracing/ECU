
# Phasing Test
##### 23/07/2022

______________
##### General 
-Phasing is the process in which the motor rotates 360 degrees in the CW direction once and we can determine the feedback offset angle
-4 12V batteries in series connection were used to provide 48V(high voltage) to do the phasing process. 
-RFE and RUN signal were given to the motorcontroller by attaching external push buttons

![BamocarConnections](https://user-images.githubusercontent.com/93036845/184670133-faa320b2-8469-42ef-80a9-63e0c4e41fbf.jpg)


Steps:
1. Check that the switch in branch K4(discharge relay) is closed and has been closed for **at least 15 seconds**, once it has been closed for 15 sec, open the switch 

2. Changes in the N-Drive software
    -  Set Speed 2 at Pos-Reference to 100 
    -  Set Inom eff in settings to 20A
    -  Set Mpole in settings to 20
    -  Set FB pole in settings to 2
    -  Set Mtemp to 32767(to resolve the motor temp issue value and for actual working, we get the calculation from DT of around)
    -  Set Imax pk to 3%
3. Give the RFE signal {It is mentioned in one of the emails that this can be done before applying the HV }
4. Close the switch in K3(negative AIR)
5. To initial Pre-Charge, close the switch in branch K2(Precharge relay)
6. Using the N-Drive Software determine when the DC-BUS is at least 90% of the total applied voltage (Check NDrive->Extra->Vdc-Bus (0xeb).
7. Once the DC-BUS voltage is **at least 90%** , close the switch in branch K1(Positive AIR) [It was not charged exactly to 90% but was actually showing around 50-51V even though the supply was only 48V]
8. Once the switch in branch K1(positive AIR) has been closed, open the switch in branch K2(Precharge)
9. **High voltage is now being supplied to the motor controller and the system is considered live and should be handled carefully and cautiously 
10.  Changes in N-Drive software
     - Auto ~> Special functions ~> select [fn4] Phasing-Rotating
     - Press START (**The next step must be done before 10 s are over) 
     -  Switch on X1-G (RUN: Run enable)
11. The motor must then rotate 360° once in CW direction. 
12. Disable drive by switching X1-G  (RUN) to off (NDrive will tell you) [ Please always deactivate the inverter with the RUN signal before cutting the HV voltage. Even at 48V.The RFE does not need to be disconnected. Disconnecting the RUN input is enough.]


 [First time testing videos](https://drive.google.com/drive/folders/1Byp8bGNmG__OxLTApcdZgOM1uu7ePpO7 "With a Title")

### Issues Faced
1. As seen in the EMRAX-RS_V2 Resolver connector pdf , the wire in the 6th pin of the Feedback plug motor is black-white but no black-white wire is there [Resolved(1)]
2. After completing phasing, the N-drive software is supposed to give back the offset angle of our motor, but it gives a garbage value/whatever value you manually set in it
3. As mentioned in the initialisation guide the motor is supposed to run during phasing at an Imax pk value of 5% it should draw a current of 2-3A {mentioned in emails by edward} however at 5% it draws 21.2A {as shown in phasing videos} and motor does not run at 5%[Resolved(3)]
4. The RS232 serial interface requires a connector from M8 to D9-S [Resolved(2)]
	   
![1](https://user-images.githubusercontent.com/93036845/184670173-512488b0-ff51-4e91-8855-7a92d070d92c.jpeg)



### Issues Resolved
1. There were translation issues and the wire is supposed to be yellow-white
2. We have custom made the connector ourselves by soldering the resistor as well as the connections(ask dhaval)
3. We set the Imax pk value to 3% during our successfull test and the current drawn was 12.7A(as shown in N-drive software)

### Circuit Used
1. The normal connections for battery pack(High voltage) connections are replaced by the 4 12V EXIDE batteries 
2. The controlling of the positive AIR which originally is done by a signal from the MASTER PCB(once 90% charging is done of the motor controller capacitor) was replaced by and external push button 
3. The RS232 connector needs special drivers 

![2](https://user-images.githubusercontent.com/93036845/184670253-423d9065-19a5-474b-823a-5939ba802217.jpeg)
![3](https://user-images.githubusercontent.com/93036845/184670304-ee1a91f6-4d5c-441c-9631-65b94687c3a1.jpeg)
![4](https://user-images.githubusercontent.com/93036845/184670330-64361df3-2701-4bed-a409-e050728879a1.jpeg)

