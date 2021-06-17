# LG Security Specialist (Team2) - Studio Project


1. Exploring Directories

    /**docs**
      1) Team2_Project_Phase1_Presentation.pptx : presentation material for Phase 1 including user guide

      2) Team2_Project_2021-06-17.pptx : Studio project documentation

      2) Team2_HowToBuildClient.pptx : Client Build guide
      3) Team2_TestCases.xlsx : Test cases
      4) Team2_static_analysis_FlawFinder.xlsx :  code review & known defects 

    /**src**
    1) LgFaceRecDemoTCP_Jetson_NanoV2 : source files GSS ( server side)

    > **prerequisite** : openssl-1.1.1k is required to install on Jetson Nano.
    >
    > 1) go to https://github.com/openssl/openssl and get openssl 1.1.1k
    >
    > 2) unzip the package
    >
    > 3) > ./config
    >
    > 4) > make
    >
    > 5) > make install 
    >
    > 6) Add  below to ~/.bashrc
    >
    >  *export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH* 
    >
    > 7) >  source ~/.bashrc 
    >
    > 8) >sudo ldconfig 

    2) Security : source files of GCS ( client side) based on Visual Studio

    

2. Known Defects

    1)  When the server is running as non-secure mode and the client tries to connect to server as secure mode, it causes hang on both sides.

    2) If you want to change the communication mode ( secure/non-secure mode) of each application,  they need to be terminated and restarted.

    3) Creating new User ID, removing user ID and resetting password are not implemented.     

    

3. User ID/PW : 

    1) General user : kinduser / lgssteam2
    2) Administrator : admin / wewant2go2cmu

    

4. Contact info
    E-mail : lg-security-specialist-team2@googlegroups.com

