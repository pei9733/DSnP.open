cirGate.o: cirGate.cpp cirGate.h cirDef.h cirMgr.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
cirMgr.o: cirMgr.cpp cirMgr.h cirGate.h cirDef.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h CirGate.h
cirCmd.o: cirCmd.cpp cirMgr.h cirGate.h cirDef.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h cirCmd.h \
 ../../include/cmdParser.h ../../include/cmdCharDef.h
