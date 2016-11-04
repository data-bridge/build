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
	Files.cpp		\
	Group.cpp		\
        Location.cpp            \
	Play.cpp		\
	Players.cpp		\
        Scoring.cpp             \
        Segment.cpp             \
        Session.cpp             \
	Tableau.cpp		\
        Team.cpp                \
        Teams.cpp               \
        Timer.cpp               \
        Timers.cpp              \
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
	ValProfile.cpp		\
	ValStats.cpp		\
	validate.cpp		\
	validateLIN.cpp		\
	validatePBN.cpp		\
	validateRBN.cpp		\
	validateTXT.cpp		\
	validateEML.cpp		\
	validateREC.cpp		\
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
# LD_FLAGS_VS	= /INCREMENTAL:NO /DEBUG

WINDRES_FLAG	= -F pe-i386

CC		= $(CC_VS)
CC_FLAGS	= $(CC_FLAGS_VS) $(WARN_FLAGS)
# CC_FLAGS	= $(CC_FLAGS_DEBUG) $(WARN_FLAGS) /Zi
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

Auction.obj: Auction.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Bexcept.obj: Bexcept.h
Bdiff.obj: Bdiff.h
Board.obj: bconst.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Board.obj: Play.h Valuation.h parse.h Bexcept.h Bdiff.h
Canvas.obj: Canvas.h
Contract.obj: Contract.h bconst.h parse.h Bexcept.h Bdiff.h
Date.obj: Date.h bconst.h parse.h Bexcept.h Bdiff.h
Deal.obj: Deal.h bconst.h parse.h Bexcept.h Bdiff.h
Files.obj: Files.h bconst.h parse.h
Group.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
Group.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
Group.obj: Contract.h Play.h Bdiff.h
Location.obj: Location.h bconst.h Bexcept.h Bdiff.h
Play.obj: Play.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Players.obj: Players.h bconst.h parse.h Bexcept.h Bdiff.h
Scoring.obj: Scoring.h bconst.h Bexcept.h Bdiff.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Team.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Segment.obj: Play.h parse.h Bexcept.h Bdiff.h
Session.obj: Session.h bconst.h parse.h Bdiff.h Bexcept.h
Tableau.obj: Tableau.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Team.obj: Team.h bconst.h parse.h Bexcept.h Bdiff.h
Teams.obj: Teams.h Team.h bconst.h parse.h Bexcept.h Bdiff.h
Timer.obj: Timer.h
Timers.obj: Timers.h Timer.h bconst.h
Valuation.obj: Valuation.h
args.obj: args.h bconst.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
dispatch.obj: Auction.h Contract.h Play.h dispatch.h Files.h Timers.h Timer.h
dispatch.obj: ValStats.h ValProfile.h validate.h fileLIN.h filePBN.h
dispatch.obj: fileRBN.h fileTXT.h fileEML.h fileREC.h parse.h Bexcept.h
dispatch.obj: Bdiff.h
fileEML.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileEML.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileEML.obj: Contract.h Play.h Canvas.h fileEML.h parse.h
filePBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
filePBN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
filePBN.obj: Contract.h Play.h filePBN.h Bexcept.h
fileRBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileRBN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileRBN.obj: Contract.h Play.h Bexcept.h fileRBN.h parse.h
fileTXT.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileTXT.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileTXT.obj: Contract.h Play.h Canvas.h fileTXT.h parse.h
fileLIN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileLIN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileLIN.obj: Contract.h Play.h fileLIN.h Bexcept.h
fileREC.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileREC.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileREC.obj: Contract.h Play.h Canvas.h fileREC.h parse.h Bexcept.h
parse.obj: parse.h bconst.h
ValProfile.obj: ValProfile.h valint.h
ValStats.obj: ValStats.h ValProfile.h bconst.h valint.h
validate.obj: ValStats.h ValProfile.h bconst.h validate.h valint.h
validate.obj: validateLIN.h validatePBN.h validateRBN.h validateTXT.h
validate.obj: validateEML.h validateREC.h parse.h Bexcept.h
validateLIN.obj: validate.h ValStats.h ValProfile.h bconst.h valint.h
validateLIN.obj: validateLIN.h parse.h
validatePBN.obj: validate.h ValStats.h ValProfile.h bconst.h valint.h
validatePBN.obj: validatePBN.h parse.h
validateRBN.obj: ValProfile.h validate.h ValStats.h bconst.h valint.h
validateRBN.obj: validateRBN.h parse.h
validateTXT.obj: validate.h ValStats.h ValProfile.h bconst.h valint.h
validateTXT.obj: validateTXT.h parse.h
validateEML.obj: validate.h ValStats.h ValProfile.h bconst.h valint.h
validateEML.obj: validateEML.h parse.h
validateREC.obj: validate.h ValStats.h ValProfile.h bconst.h valint.h
validateREC.obj: validateREC.h parse.h
reader.obj: args.h bconst.h dispatch.h Files.h Timers.h Timer.h ValStats.h
reader.obj: ValProfile.h
reader.obj: args.h bconst.h dispatch.h Files.h Timers.h Timer.h ValStats.h
reader.obj: ValProfile.h
