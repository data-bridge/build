EXPORTER	= Exports.def

SOURCE_FILES 	=		\
	Auction.cpp		\
        Bexcept.cpp             \
        Bdiff.cpp               \
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
	dispatch.cpp		\
	fileEML.cpp		\
	filePBN.cpp		\
	fileRBN.cpp		\
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
Bexcept.obj: Bexcept.h
Board.obj: bconst.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Board.obj: Play.h Debug.h Valuation.h portab.h parse.h
Canvas.obj: Canvas.h
Contract.obj: Contract.h bconst.h Debug.h parse.h portab.h
Date.obj: Date.h bconst.h portab.h parse.h Debug.h
Deal.obj: Deal.h bconst.h Debug.h parse.h
Debug.obj: Debug.h
Files.obj: dirent.h Files.h bconst.h Debug.h portab.h parse.h
Group.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
Group.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Group.obj: Play.h portab.h parse.h Debug.h
Location.obj: Location.h bconst.h portab.h Debug.h
Play.obj: Play.h bconst.h Contract.h Debug.h parse.h portab.h
Players.obj: Players.h bconst.h portab.h parse.h Debug.h
Scoring.obj: Scoring.h bconst.h portab.h Debug.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Board.h Deal.h Tableau.h Players.h Auction.h Contract.h Play.h
Segment.obj: portab.h parse.h Debug.h
Session.obj: Session.h bconst.h Debug.h portab.h parse.h
Tableau.obj: Tableau.h bconst.h Contract.h Debug.h parse.h
Teams.obj: Teams.h bconst.h portab.h parse.h Debug.h
Valuation.obj: Valuation.h
args.obj: bconst.h args.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Board.h Deal.h Tableau.h Players.h Auction.h
dispatch.obj: Contract.h Play.h dispatch.h Files.h fileLIN.h filePBN.h
dispatch.obj: fileRBN.h fileTXT.h fileEML.h fileREC.h parse.h portab.h
dispatch.obj: Bexcept.h Debug.h
fileEML.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileEML.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
fileEML.obj: Play.h Canvas.h fileEML.h parse.h Debug.h
filePBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
filePBN.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
filePBN.obj: Play.h filePBN.h portab.h Debug.h
fileRBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileRBN.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
fileRBN.obj: Play.h fileRBN.h Debug.h
fileTXT.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileTXT.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
fileTXT.obj: Play.h Canvas.h fileTXT.h parse.h Debug.h
fileLIN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileLIN.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
fileLIN.obj: Play.h fileLIN.h Debug.h
fileREC.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileREC.obj: Teams.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
fileREC.obj: Play.h Canvas.h fileREC.h parse.h Debug.h
parse.obj: Debug.h parse.h bconst.h portab.h
reader.obj: args.h dispatch.h Files.h bconst.h Debug.h
reader.obj: args.h dispatch.h Files.h bconst.h Debug.h
