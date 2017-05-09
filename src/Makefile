EXPORTER	= Exports.def

SOURCE_FILES 	=		\
	Auction.cpp		\
        Bdiff.cpp               \
        Bexcept.cpp             \
        Board.cpp               \
	Buffer.cpp		\
	Canvas.cpp		\
	Chunk.cpp		\
	CompStats.cpp		\
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
	Sheet.cpp		\
	SheetHand.cpp		\
	Tableau.cpp		\
        Team.cpp                \
        Teams.cpp               \
	TextStats.cpp		\
        Timer.cpp               \
        Timers.cpp              \
	ValProfile.cpp		\
	ValStats.cpp		\
	Valuation.cpp		\
	args.cpp		\
	ddsIF.cpp		\
	dispatch.cpp		\
	fileEML.cpp		\
	filePBN.cpp		\
	fileRBN.cpp		\
	fileTXT.cpp		\
	fileLIN.cpp		\
	fileREC.cpp		\
	heurfix.cpp		\
	parse.cpp		\
	referr.cpp		\
	reflineGen.cpp		\
	reflineLIN.cpp		\
	reflinePBN.cpp		\
	reflineRBN.cpp		\
	reflineTXT.cpp		\
	validate.cpp		\
	validateLIN.cpp		\
	validatePBN.cpp		\
	validateRBN.cpp		\
	validateTXT.cpp		\
	validateEML.cpp		\
	validateREC.cpp		\
	reader.cpp

DLIB		= dds.lib

TEST		= reader

CC_VS     	= cl

WARN_FLAGS      =               \
        /Wall                   \
        /wd4061                 \
        /wd4091                 \
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
# CC_FLAGS_VS     = /O2 /Oi /Ot /Oy /GL /Zi /EHa /DEBUG
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
	link /LTCG $(LD_FLAGS) $(OBJ_FILES) $(DLIB) /out:$(TEST).exe

%.obj:	%.cpp
	$(CC) $(CC_FLAGS) /c $<


depend:
	makedepend -Y -o.obj -- $(CC_FLAGS) -- $(SOURCE_FILES) $(TEST).cpp

clean:
	rm -f $(OBJ_FILES) $(TEST_OBJ_FILES) $(TEST).obj $(TEST).exe 


# DO NOT DELETE

Auction.obj: Auction.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Bdiff.obj: Bdiff.h
Bexcept.obj: Bexcept.h
Board.obj: bconst.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Board.obj: Play.h ddsIF.h dll.h Valuation.h parse.h Bexcept.h Bdiff.h
Buffer.obj: Buffer.h valint.h referr.h bconst.h parse.h Bexcept.h
Canvas.obj: Canvas.h
Chunk.obj: Chunk.h bconst.h parse.h Bexcept.h Bdiff.h
CompStats.obj: CompStats.h bconst.h
Contract.obj: Contract.h bconst.h parse.h Bexcept.h Bdiff.h
Date.obj: Date.h bconst.h parse.h Bexcept.h Bdiff.h
Deal.obj: Deal.h bconst.h parse.h Bexcept.h Bdiff.h
Files.obj: Files.h bconst.h parse.h
Group.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
Group.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
Group.obj: Contract.h Play.h ddsIF.h dll.h Bdiff.h
Location.obj: Location.h bconst.h Bexcept.h Bdiff.h
Play.obj: Play.h bconst.h ddsIF.h dll.h Contract.h parse.h Bexcept.h Bdiff.h
Players.obj: Players.h bconst.h parse.h Bexcept.h Bdiff.h
Scoring.obj: Scoring.h bconst.h Bexcept.h Bdiff.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Team.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Segment.obj: Play.h ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Session.obj: Session.h bconst.h parse.h Bdiff.h Bexcept.h
Sheet.obj: Buffer.h valint.h referr.h bconst.h Segment.h Date.h Location.h
Sheet.obj: Session.h Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
Sheet.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h Sheet.h
Sheet.obj: SheetHand.h parse.h Bexcept.h Bdiff.h
SheetHand.obj: SheetHand.h Contract.h bconst.h Deal.h Auction.h Play.h
SheetHand.obj: ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Tableau.obj: Tableau.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Team.obj: Team.h bconst.h parse.h Bexcept.h Bdiff.h
Teams.obj: Teams.h Team.h bconst.h parse.h Bexcept.h Bdiff.h
TextStats.obj: TextStats.h bconst.h parse.h
Timer.obj: Timer.h
Timers.obj: Timers.h Timer.h bconst.h
ValProfile.obj: ValProfile.h validate.h Buffer.h valint.h referr.h bconst.h
ValStats.obj: ValStats.h ValProfile.h validate.h Buffer.h valint.h referr.h
ValStats.obj: bconst.h
Valuation.obj: Valuation.h
args.obj: args.h bconst.h
ddsIF.obj: ddsIF.h dll.h Bexcept.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
dispatch.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Chunk.h dispatch.h
dispatch.obj: Files.h Timers.h Timer.h ValStats.h ValProfile.h validate.h
dispatch.obj: Buffer.h valint.h referr.h TextStats.h CompStats.h heurfix.h
dispatch.obj: fileLIN.h filePBN.h fileRBN.h fileTXT.h fileEML.h fileREC.h
dispatch.obj: parse.h Sheet.h SheetHand.h Bexcept.h Bdiff.h
fileEML.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileEML.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileEML.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileEML.h Buffer.h
fileEML.obj: valint.h referr.h Chunk.h parse.h Bexcept.h
filePBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
filePBN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
filePBN.obj: Contract.h Play.h ddsIF.h dll.h filePBN.h Buffer.h valint.h
filePBN.obj: referr.h Chunk.h Bexcept.h
fileRBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileRBN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileRBN.obj: Contract.h Play.h ddsIF.h dll.h Bexcept.h fileRBN.h Buffer.h
fileRBN.obj: valint.h referr.h Chunk.h parse.h
fileTXT.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileTXT.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileTXT.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileTXT.h Buffer.h
fileTXT.obj: valint.h referr.h Chunk.h parse.h Bexcept.h
fileLIN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileLIN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileLIN.obj: Contract.h Play.h ddsIF.h dll.h fileLIN.h Buffer.h valint.h
fileLIN.obj: referr.h Chunk.h parse.h Bexcept.h
fileREC.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileREC.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileREC.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileREC.h Buffer.h
fileREC.obj: valint.h referr.h Chunk.h parse.h Bexcept.h
heurfix.obj: heurfix.h Group.h Segment.h Date.h bconst.h Location.h Session.h
heurfix.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
heurfix.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Buffer.h valint.h
heurfix.obj: referr.h Chunk.h parse.h Bexcept.h
parse.obj: parse.h bconst.h
referr.obj: referr.h reflineGen.h reflineLIN.h reflinePBN.h reflineRBN.h
referr.obj: reflineTXT.h parse.h bconst.h Bexcept.h
reflineGen.obj: reflineGen.h referr.h parse.h bconst.h Bexcept.h
reflineLIN.obj: reflineLIN.h referr.h parse.h bconst.h Bexcept.h
reflinePBN.obj: reflinePBN.h referr.h parse.h bconst.h Bexcept.h
reflineRBN.obj: reflineRBN.h referr.h parse.h bconst.h Bexcept.h
reflineTXT.obj: reflineTXT.h referr.h parse.h bconst.h Bexcept.h
validate.obj: ValStats.h ValProfile.h validate.h Buffer.h valint.h referr.h
validate.obj: bconst.h filePBN.h Segment.h Date.h Location.h Session.h
validate.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
validate.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Chunk.h validateLIN.h
validate.obj: validatePBN.h validateRBN.h validateTXT.h validateEML.h
validate.obj: validateREC.h parse.h
validateLIN.obj: Deal.h bconst.h validateLIN.h ValProfile.h validate.h
validateLIN.obj: Buffer.h valint.h referr.h parse.h Bexcept.h Bdiff.h
validatePBN.obj: parse.h bconst.h validatePBN.h ValProfile.h validate.h
validatePBN.obj: Buffer.h valint.h referr.h Chunk.h
validateRBN.obj: validateRBN.h ValProfile.h validate.h Buffer.h valint.h
validateRBN.obj: referr.h bconst.h parse.h
validateTXT.obj: validateTXT.h ValProfile.h validate.h Buffer.h valint.h
validateTXT.obj: referr.h bconst.h parse.h
validateEML.obj: validateEML.h ValProfile.h validate.h Buffer.h valint.h
validateEML.obj: referr.h bconst.h
validateREC.obj: validateREC.h ValProfile.h validate.h Buffer.h valint.h
validateREC.obj: referr.h bconst.h parse.h
reader.obj: args.h bconst.h dispatch.h Files.h Timers.h Timer.h ValStats.h
reader.obj: ValProfile.h validate.h Buffer.h valint.h referr.h TextStats.h
reader.obj: CompStats.h
reader.obj: args.h bconst.h dispatch.h Files.h Timers.h Timer.h ValStats.h
reader.obj: ValProfile.h validate.h Buffer.h valint.h referr.h TextStats.h
reader.obj: CompStats.h
