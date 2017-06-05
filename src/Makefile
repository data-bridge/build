EXPORTER	= Exports.def

SOURCE_FILES 	=		\
	AllStats.cpp		\
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
	funcCompare.cpp		\
	funcDigest.cpp		\
	funcIMPSheet.cpp	\
	funcPlayerVal.cpp	\
	funcRead.cpp		\
	funcRefStats.cpp	\
	funcTextStats.cpp	\
	funcValidate.cpp	\
	funcWrite.cpp		\
	parse.cpp		\
	OrderCounts.cpp		\
	RefAction.cpp		\
	RefComment.cpp		\
	RefEdit.cpp		\
	RefLine.cpp		\
	RefLines.cpp		\
	RefStats.cpp		\
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

AllStats.obj: AllStats.h ValStats.h ValProfile.h validate.h Buffer.h valint.h
AllStats.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
AllStats.obj: RefAction.h TextStats.h CompStats.h RefStats.h Timers.h Timer.h
Auction.obj: Auction.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Bdiff.obj: Bdiff.h
Bexcept.obj: Bexcept.h
Board.obj: bconst.h Board.h Deal.h Tableau.h Players.h Auction.h Contract.h
Board.obj: Play.h ddsIF.h dll.h Valuation.h parse.h Bexcept.h Bdiff.h
Buffer.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
Buffer.obj: RefComment.h bconst.h RefAction.h parse.h Bexcept.h
Canvas.obj: Canvas.h
Chunk.obj: Chunk.h bconst.h parse.h Bexcept.h Bdiff.h
CompStats.obj: CompStats.h bconst.h parse.h
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
Sheet.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
Sheet.obj: RefComment.h bconst.h RefAction.h Segment.h Date.h Location.h
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
ValProfile.obj: ValProfile.h validate.h Buffer.h valint.h Reflines.h
ValProfile.obj: RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
ValProfile.obj: RefAction.h
ValStats.obj: ValStats.h ValProfile.h validate.h Buffer.h valint.h Reflines.h
ValStats.obj: RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
ValStats.obj: RefAction.h
Valuation.obj: Valuation.h
args.obj: args.h bconst.h
ddsIF.obj: ddsIF.h dll.h Bexcept.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
dispatch.obj: Auction.h Contract.h Play.h ddsIF.h dll.h dispatch.h Files.h
dispatch.obj: AllStats.h ValStats.h ValProfile.h validate.h Buffer.h valint.h
dispatch.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
dispatch.obj: RefAction.h TextStats.h CompStats.h RefStats.h Timers.h Timer.h
dispatch.obj: funcCompare.h funcDigest.h funcIMPSheet.h funcRead.h
dispatch.obj: funcPlayerVal.h funcRefStats.h RefLines.h funcTextStats.h
dispatch.obj: funcValidate.h funcWrite.h
fileEML.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileEML.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileEML.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileEML.h Buffer.h
fileEML.obj: valint.h Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
fileEML.obj: RefAction.h Chunk.h parse.h Bexcept.h
filePBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
filePBN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
filePBN.obj: Contract.h Play.h ddsIF.h dll.h filePBN.h Buffer.h valint.h
filePBN.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
filePBN.obj: RefAction.h Chunk.h Bexcept.h
fileRBN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileRBN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileRBN.obj: Contract.h Play.h ddsIF.h dll.h Bexcept.h fileRBN.h Buffer.h
fileRBN.obj: valint.h Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
fileRBN.obj: RefAction.h Chunk.h parse.h
fileTXT.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileTXT.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileTXT.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileTXT.h Buffer.h
fileTXT.obj: valint.h Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
fileTXT.obj: RefAction.h Chunk.h parse.h Bexcept.h
fileLIN.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileLIN.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileLIN.obj: Contract.h Play.h ddsIF.h dll.h fileLIN.h Buffer.h valint.h
fileLIN.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
fileLIN.obj: RefAction.h Chunk.h parse.h Bexcept.h
fileREC.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
fileREC.obj: Teams.h Team.h Board.h Deal.h Tableau.h Players.h Auction.h
fileREC.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileREC.h Buffer.h
fileREC.obj: valint.h Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
fileREC.obj: RefAction.h Chunk.h parse.h Bexcept.h
funcCompare.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcCompare.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
funcCompare.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Buffer.h valint.h
funcCompare.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
funcCompare.obj: RefAction.h funcCompare.h CompStats.h funcRead.h Bexcept.h
funcCompare.obj: Bdiff.h
funcDigest.obj: funcDigest.h bconst.h Sheet.h Contract.h Deal.h Auction.h
funcDigest.obj: Play.h ddsIF.h dll.h Buffer.h valint.h Reflines.h RefLine.h
funcDigest.obj: RefEdit.h refconst.h RefComment.h RefAction.h SheetHand.h
funcDigest.obj: parse.h Bexcept.h
funcIMPSheet.obj: funcIMPSheet.h Group.h Segment.h Date.h bconst.h Location.h
funcIMPSheet.obj: Session.h Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
funcIMPSheet.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h Sheet.h
funcIMPSheet.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
funcIMPSheet.obj: RefComment.h RefAction.h SheetHand.h Bexcept.h
funcPlayerVal.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcPlayerVal.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
funcPlayerVal.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcPlayerVal.obj: funcPlayerVal.h Bexcept.h
funcRead.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcRead.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
funcRead.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Chunk.h fileLIN.h
funcRead.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
funcRead.obj: RefComment.h RefAction.h filePBN.h fileRBN.h fileTXT.h
funcRead.obj: fileEML.h fileREC.h funcRead.h parse.h Bexcept.h
funcRefStats.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
funcRefStats.obj: RefComment.h bconst.h RefAction.h funcRefStats.h RefLines.h
funcRefStats.obj: RefStats.h Bexcept.h
funcTextStats.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcTextStats.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
funcTextStats.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcTextStats.obj: funcTextStats.h TextStats.h Bexcept.h
funcValidate.obj: validate.h Buffer.h valint.h Reflines.h RefLine.h RefEdit.h
funcValidate.obj: refconst.h RefComment.h bconst.h RefAction.h funcValidate.h
funcValidate.obj: ValStats.h ValProfile.h Bexcept.h
funcWrite.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcWrite.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
funcWrite.obj: Auction.h Contract.h Play.h ddsIF.h dll.h fileLIN.h Buffer.h
funcWrite.obj: valint.h Reflines.h RefLine.h RefEdit.h refconst.h
funcWrite.obj: RefComment.h RefAction.h Chunk.h filePBN.h fileRBN.h fileTXT.h
funcWrite.obj: fileEML.h fileREC.h parse.h Bexcept.h
parse.obj: parse.h bconst.h
OrderCounts.obj: OrderCounts.h bconst.h
RefAction.obj: RefAction.h refconst.h Bexcept.h
RefComment.obj: RefComment.h refconst.h bconst.h parse.h Bexcept.h
RefEdit.obj: RefEdit.h refconst.h parse.h bconst.h Bexcept.h
RefLine.obj: RefLine.h RefEdit.h refconst.h RefComment.h bconst.h RefAction.h
RefLine.obj: parse.h Bexcept.h
RefLines.obj: RefLines.h parse.h bconst.h Bexcept.h
RefStats.obj: RefStats.h refconst.h RefComment.h bconst.h Bexcept.h
validate.obj: ValStats.h ValProfile.h validate.h Buffer.h valint.h Reflines.h
validate.obj: RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
validate.obj: RefAction.h filePBN.h Segment.h Date.h Location.h Session.h
validate.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Players.h
validate.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Chunk.h validateLIN.h
validate.obj: validatePBN.h validateRBN.h validateTXT.h validateEML.h
validate.obj: validateREC.h parse.h
validateLIN.obj: Deal.h bconst.h validateLIN.h ValProfile.h validate.h
validateLIN.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
validateLIN.obj: RefComment.h RefAction.h parse.h Bexcept.h Bdiff.h
validatePBN.obj: parse.h bconst.h validatePBN.h ValProfile.h validate.h
validatePBN.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
validatePBN.obj: RefComment.h RefAction.h Chunk.h
validateRBN.obj: validateRBN.h ValProfile.h validate.h Buffer.h valint.h
validateRBN.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
validateRBN.obj: bconst.h RefAction.h parse.h
validateTXT.obj: validateTXT.h ValProfile.h validate.h Buffer.h valint.h
validateTXT.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
validateTXT.obj: bconst.h RefAction.h parse.h
validateEML.obj: validateEML.h ValProfile.h validate.h Buffer.h valint.h
validateEML.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
validateEML.obj: bconst.h RefAction.h
validateREC.obj: validateREC.h ValProfile.h validate.h Buffer.h valint.h
validateREC.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
validateREC.obj: bconst.h RefAction.h parse.h
reader.obj: args.h bconst.h AllStats.h ValStats.h ValProfile.h validate.h
reader.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
reader.obj: RefComment.h RefAction.h TextStats.h CompStats.h RefStats.h
reader.obj: Timers.h Timer.h dispatch.h Files.h
reader.obj: args.h bconst.h AllStats.h ValStats.h ValProfile.h validate.h
reader.obj: Buffer.h valint.h Reflines.h RefLine.h RefEdit.h refconst.h
reader.obj: RefComment.h RefAction.h TextStats.h CompStats.h RefStats.h
reader.obj: Timers.h Timer.h dispatch.h Files.h
