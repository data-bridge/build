EXPORTER	= Exports.def

SOURCE_FILES 	=		\
	Auction.cpp		\
        Board.cpp               \
	Contract.cpp		\
	Date.cpp		\
	Deal.cpp		\
	Debug.cpp		\
        Location.cpp            \
	Play.cpp		\
	Players.cpp		\
        Scoring.cpp             \
        Segment.cpp             \
        Session.cpp             \
	Tableau.cpp		\
        Teams.cpp               \
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
Contract.obj: Contract.h bconst.h Debug.h
Date.obj: Date.h bconst.h portab.h parse.h debug.h Debug.h
Deal.obj: bconst.h Deal.h Players.h Debug.h parse.h
Debug.obj: Debug.h
Location.obj: Location.h bconst.h portab.h debug.h Debug.h
Play.obj: Play.h bconst.h Contract.h Deal.h Players.h Debug.h parse.h
Play.obj: portab.h
Players.obj: Players.h bconst.h portab.h
Scoring.obj: Scoring.h bconst.h portab.h debug.h Debug.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Board.h Deal.h Players.h Tableau.h Contract.h Valuation.h
Segment.obj: Auction.h Play.h portab.h debug.h Debug.h
Session.obj: Session.h bconst.h portab.h parse.h debug.h Debug.h
Tableau.obj: Tableau.h bconst.h Contract.h Debug.h parse.h
Teams.obj: Teams.h bconst.h portab.h debug.h Debug.h
parse.obj: parse.h portab.h
reader.obj: portab.h bconst.h Debug.h Tableau.h Contract.h Deal.h Players.h
reader.obj: Auction.h Play.h
reader.obj: portab.h bconst.h Debug.h Tableau.h Contract.h Deal.h Players.h
reader.obj: Auction.h Play.h
