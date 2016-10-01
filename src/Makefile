EXPORTER	= Exports.def

SOURCE_FILES 	=		\
	Auction.cpp		\
        Board.cpp               \
	Canvas.cpp		\
	Contract.cpp		\
	Date.cpp		\
	Deal.cpp		\
	Debug.cpp		\
	Files.cpp		\
	Group.cpp		\
        Location.cpp            \
	Play.cpp		\
	Players.cpp		\
        Scoring.cpp             \
        Segment.cpp             \
        Session.cpp             \
	Tableau.cpp		\
        Teams.cpp               \
	Valuation.cpp		\
	args.cpp		\
	fileEML.cpp		\
	filePBN.cpp		\
	fileRBN.cpp		\
	fileRBX.cpp		\
	fileTXT.cpp		\
	fileLIN.cpp		\
	fileREC.cpp		\
	parse.cpp		\
	reader.cpp

TEST		= reader

CC_VS     	= cl

WARN_FLAGS      =               \
        /Wall                   \
        /wd4061                 \
        /wd4127                 \
        /wd4265                 \
        /wd4350                 \
        /wd4514                 \
        /wd4555                 \
        /wd4668                 \
        /wd4701                 \
        /wd4710                 \
        /wd4711                 \
        /wd4820                 \
        /wd4986                 \
        /wd4987                 \
        /wd4996                 \
        /WX

CC_FLAGS_VS     = /O2 /Oi /Ot /Oy /GL /Zi /EHsc
CC_FLAGS_DEBUG  = /DEBUG


LD_FLAGS_VS	= /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF

WINDRES_FLAG	= -F pe-i386

CC		= $(CC_VS)
CC_FLAGS	= $(CC_FLAGS_VS) $(WARN_FLAGS)
LD_FLAGS	= $(LD_FLAGS_VS)

OBJ_FILES 	= $(subst .cpp,.obj,$(SOURCE_FILES))

# Linking directly.

reader:	$(OBJ_FILES)
	link /LTCG $(LD_FLAGS) $(OBJ_FILES) /out:$(TEST).exe

%.obj:	%.cpp
	$(CC) $(CC_FLAGS) /c /EHsc $<


depend:
	makedepend -Y -o.obj -- $(CC_FLAGS) -- $(SOURCE_FILES) $(TEST).cpp

clean:
	rm -f $(OBJ_FILES) $(TEST_OBJ_FILES) $(TEST).obj $(TEST).exe 


# DO NOT DELETE

Auction.obj: Auction.h bconst.h Contract.h Debug.h parse.h portab.h
Board.obj: bconst.h Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
Board.obj: Auction.h Play.h Debug.h portab.h parse.h
Canvas.obj: Canvas.h
Contract.obj: Contract.h bconst.h Debug.h parse.h portab.h
Date.obj: Date.h bconst.h portab.h parse.h debug.h Debug.h
Deal.obj: bconst.h Deal.h Players.h Debug.h parse.h
Debug.obj: Debug.h
Files.obj: bconst.h Files.h Debug.h portab.h
Group.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
Group.obj: Teams.h Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
Group.obj: Auction.h Play.h portab.h parse.h debug.h Debug.h
Location.obj: Location.h bconst.h portab.h debug.h Debug.h
Play.obj: Play.h bconst.h Contract.h Deal.h Players.h Debug.h parse.h
Play.obj: portab.h
Players.obj: bconst.h Players.h portab.h parse.h Debug.h
Scoring.obj: Scoring.h bconst.h portab.h debug.h Debug.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
Segment.obj: Auction.h Play.h portab.h parse.h debug.h Debug.h
Session.obj: Session.h bconst.h portab.h parse.h debug.h Debug.h
Tableau.obj: Tableau.h bconst.h Contract.h Debug.h parse.h
Teams.obj: Teams.h bconst.h portab.h parse.h Debug.h
args.obj: bconst.h args.h
fileEML.obj: Canvas.h fileEML.h Group.h Segment.h Date.h bconst.h Location.h
fileEML.obj: Session.h Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h
fileEML.obj: Contract.h Valuation.h Auction.h Play.h parse.h portab.h debug.h
fileEML.obj: Debug.h
filePBN.obj: filePBN.h Group.h Segment.h Date.h bconst.h Location.h Session.h
filePBN.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
filePBN.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
fileRBN.obj: fileRBN.h Group.h Segment.h Date.h bconst.h Location.h Session.h
fileRBN.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
fileRBN.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
fileRBX.obj: fileRBX.h Group.h Segment.h Date.h bconst.h Location.h Session.h
fileRBX.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
fileRBX.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
fileTXT.obj: Canvas.h fileTXT.h Group.h Segment.h Date.h bconst.h Location.h
fileTXT.obj: Session.h Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h
fileTXT.obj: Contract.h Valuation.h Auction.h Play.h parse.h portab.h debug.h
fileTXT.obj: Debug.h
fileLIN.obj: fileLIN.h Group.h Segment.h Date.h bconst.h Location.h Session.h
fileLIN.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
fileLIN.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
fileREC.obj: Canvas.h fileREC.h Group.h Segment.h Date.h bconst.h Location.h
fileREC.obj: Session.h Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h
fileREC.obj: Contract.h Valuation.h Auction.h Play.h parse.h portab.h debug.h
fileREC.obj: Debug.h
parse.obj: bconst.h parse.h portab.h debug.h Debug.h
reader.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
reader.obj: Teams.h Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
reader.obj: Auction.h Play.h args.h fileRBN.h fileRBX.h filePBN.h
reader.obj: fileEML.h fileTXT.h fileLIN.h fileREC.h portab.h Debug.h
reader.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
reader.obj: Teams.h Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
reader.obj: Auction.h Play.h args.h fileRBN.h fileRBX.h filePBN.h
reader.obj: fileEML.h fileTXT.h fileLIN.h fileREC.h portab.h Debug.h
