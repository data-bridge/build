EXPORTER	= Exports.def

SOURCE_FILES 	=		\
	Auction.cpp		\
        Board.cpp               \
	Canvas.cpp		\
	Contract.cpp		\
	Date.cpp		\
	Deal.cpp		\
	Debug.cpp		\
	Group.cpp		\
        Location.cpp            \
	Play.cpp		\
	Players.cpp		\
        Scoring.cpp             \
        Segment.cpp             \
        Session.cpp             \
	Tableau.cpp		\
        Teams.cpp               \
	UnitTest.cpp		\
	fileEML.cpp		\
	filePBN.cpp		\
	fileRBN.cpp		\
	parse.cpp		\
	reader.cpp

TEST		= reader

CC_VS     	= cl

WARN_FLAGS      =               \
        /Wall                   \
        /wd4061                 \
        /wd4127                 \
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
Board.obj: Auction.h Play.h Debug.h portab.h
Canvas.obj: Canvas.h
Contract.obj: Contract.h bconst.h Debug.h parse.h portab.h
Date.obj: Date.h bconst.h portab.h parse.h debug.h Debug.h
Deal.obj: bconst.h Deal.h Players.h Debug.h parse.h
Debug.obj: Debug.h
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
UnitTest.obj: portab.h bconst.h UnitTest.h Tableau.h Contract.h Deal.h
UnitTest.obj: Players.h Auction.h Play.h Date.h Location.h Scoring.h Teams.h
UnitTest.obj: Session.h Segment.h Board.h Valuation.h Debug.h
fileEML.obj: fileEML.h Group.h Segment.h Date.h bconst.h Location.h Session.h
fileEML.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
fileEML.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
filePBN.obj: filePBN.h Group.h Segment.h Date.h bconst.h Location.h Session.h
filePBN.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
filePBN.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
fileRBN.obj: fileRBN.h Group.h Segment.h Date.h bconst.h Location.h Session.h
fileRBN.obj: Scoring.h Teams.h Board.h Deal.h Players.h Tableau.h Contract.h
fileRBN.obj: Valuation.h Auction.h Play.h parse.h portab.h debug.h Debug.h
parse.obj: bconst.h parse.h portab.h
reader.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
reader.obj: Teams.h Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
reader.obj: Auction.h Play.h UnitTest.h fileRBN.h filePBN.h fileEML.h
reader.obj: portab.h Debug.h
reader.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
reader.obj: Teams.h Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
reader.obj: Auction.h Play.h UnitTest.h fileRBN.h filePBN.h fileEML.h
reader.obj: portab.h Debug.h
