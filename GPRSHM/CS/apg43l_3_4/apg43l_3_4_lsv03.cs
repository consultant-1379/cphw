############################################################################## 
#
# CPHW/APG GPRSHM - CS used for development work 
# 
############################################################################## 
  
element * CHECKEDOUT
  
## CQS ## 
element /vobs/cqs/... FTM_2_3_3_T16_R1A01 
 
#SDK (AD_SDK, ACE, BOOST, CMW, COM, LDE, DX, SEC) 
element * SDK_3_2_0 
 
#For MODELS 
element * .../io_apg43L_34z/LATEST 
 
#For Tools 
element * AXE17A_CI 
element * AXE18A_TOOLS_1 
element * AXE17A_TOOLS_1_3 

## CPHW/APG

element /vobs/HW/Products/APG/GPRSHM/CS/... /main/LATEST

#element /vobs/HW/... .../<your_task_branch>/LATEST
#mkbranch <your_task_branch>
element /vobs/HW/... AXE18A_PROJ_LSV03 
element /vobs/HW/... CPHW_CAA146650_P1F0101
element /vobs/HW/... /main/0
#end mkbranch

element * .../apg43l_3_4/LATEST
element * .../io_apg43L/LATEST -mkbranch apg43l_3_4
element * /main/LATEST -mkbranch io_apg43L


element * -none 
 
############################################################################## 

