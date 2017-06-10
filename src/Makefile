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
	GivenScore.cpp		\
	Group.cpp		\
	Instance.cpp		\
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

AllStats.obj: AllStats.h ValStats.h ValProfile.h bconst.h TextStats.h
AllStats.obj: CompStats.h RefStats.h refconst.h Timers.h Timer.h
Auction.obj: Auction.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Bdiff.obj: Bdiff.h
Bexcept.obj: Bexcept.h
Board.obj: bconst.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
Board.obj: Contract.h Play.h ddsIF.h dll.h Valuation.h parse.h Bexcept.h
Board.obj: Bdiff.h
Buffer.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
Buffer.obj: RefAction.h Buffer.h parse.h Bexcept.h
Canvas.obj: Canvas.h
Chunk.obj: Chunk.h bconst.h parse.h Bexcept.h Bdiff.h
CompStats.obj: CompStats.h bconst.h parse.h
Contract.obj: Contract.h bconst.h parse.h Bexcept.h Bdiff.h
Date.obj: Date.h bconst.h parse.h Bexcept.h Bdiff.h
Deal.obj: Deal.h bconst.h parse.h Bexcept.h Bdiff.h
Files.obj: Files.h bconst.h parse.h
GivenScore.obj: bconst.h GivenScore.h parse.h Bexcept.h Bdiff.h
Group.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
Group.obj: Teams.h Team.h Board.h Deal.h Tableau.h Instance.h Players.h
Group.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Bdiff.h
Instance.obj: Instance.h Players.h bconst.h Auction.h Contract.h Play.h
Instance.obj: ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Location.obj: Location.h bconst.h Bexcept.h Bdiff.h
Play.obj: Play.h bconst.h ddsIF.h dll.h Contract.h parse.h Bexcept.h Bdiff.h
Players.obj: Players.h bconst.h parse.h Bexcept.h Bdiff.h
Scoring.obj: Scoring.h bconst.h Bexcept.h Bdiff.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
Segment.obj: Contract.h Play.h ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Session.obj: Session.h bconst.h parse.h Bdiff.h Bexcept.h
Sheet.obj: Buffer.h bconst.h Segment.h Date.h Location.h Session.h Scoring.h
Sheet.obj: Teams.h Team.h Board.h Deal.h Tableau.h Instance.h Players.h
Sheet.obj: Auction.h Contract.h Play.h ddsIF.h dll.h Sheet.h SheetHand.h
Sheet.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h RefAction.h
Sheet.obj: parse.h Bexcept.h Bdiff.h
SheetHand.obj: SheetHand.h Contract.h bconst.h Deal.h Auction.h Play.h
SheetHand.obj: ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Tableau.obj: Tableau.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Team.obj: Team.h bconst.h parse.h Bexcept.h Bdiff.h
Teams.obj: Teams.h Team.h bconst.h parse.h Bexcept.h Bdiff.h
TextStats.obj: TextStats.h bconst.h parse.h
Timer.obj: Timer.h
Timers.obj: Timers.h Timer.h bconst.h
ValProfile.obj: ValProfile.h bconst.h
ValStats.obj: ValStats.h ValProfile.h bconst.h
Valuation.obj: Valuation.h
args.obj: args.h bconst.h
ddsIF.obj: ddsIF.h dll.h Bexcept.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Instance.h
dispatch.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h dispatch.h
dispatch.obj: validate.h Files.h AllStats.h ValStats.h ValProfile.h
dispatch.obj: TextStats.h CompStats.h RefStats.h refconst.h Timers.h Timer.h
dispatch.obj: RefLines.h RefLine.h RefEdit.h RefComment.h RefAction.h
dispatch.obj: funcCompare.h funcDigest.h funcIMPSheet.h funcRead.h
dispatch.obj: funcPlayerVal.h funcRefStats.h funcTextStats.h funcValidate.h
dispatch.obj: funcWrite.h
fileEML.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileEML.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
fileEML.obj: Contract.h Play.h ddsIF.h dll.h Buffer.h Chunk.h Canvas.h
fileEML.obj: fileEML.h parse.h Bexcept.h
filePBN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
filePBN.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
filePBN.obj: Contract.h Play.h ddsIF.h dll.h Buffer.h Chunk.h filePBN.h
filePBN.obj: Bexcept.h
fileRBN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileRBN.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
fileRBN.obj: Contract.h Play.h ddsIF.h dll.h Buffer.h Chunk.h fileRBN.h
fileRBN.obj: parse.h Bexcept.h
fileTXT.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileTXT.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
fileTXT.obj: Contract.h Play.h ddsIF.h dll.h Canvas.h fileTXT.h Buffer.h
fileTXT.obj: Chunk.h parse.h Bexcept.h
fileLIN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileLIN.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
fileLIN.obj: Contract.h Play.h ddsIF.h dll.h Buffer.h Chunk.h fileLIN.h
fileLIN.obj: parse.h Bexcept.h
fileREC.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileREC.obj: Team.h Board.h Deal.h Tableau.h Instance.h Players.h Auction.h
fileREC.obj: Contract.h Play.h ddsIF.h dll.h Buffer.h Chunk.h Canvas.h
fileREC.obj: fileREC.h parse.h Bexcept.h
funcCompare.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcCompare.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Instance.h
funcCompare.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h Buffer.h
funcCompare.obj: CompStats.h funcCompare.h funcRead.h Bexcept.h Bdiff.h
funcDigest.obj: funcDigest.h bconst.h Sheet.h Contract.h Deal.h Auction.h
funcDigest.obj: Play.h ddsIF.h dll.h Buffer.h SheetHand.h Reflines.h
funcDigest.obj: RefLine.h RefEdit.h refconst.h RefComment.h RefAction.h
funcDigest.obj: parse.h Bexcept.h
funcIMPSheet.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcIMPSheet.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
funcIMPSheet.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h
funcIMPSheet.obj: dll.h Sheet.h Buffer.h SheetHand.h Reflines.h RefLine.h
funcIMPSheet.obj: RefEdit.h refconst.h RefComment.h RefAction.h
funcIMPSheet.obj: funcIMPSheet.h Bexcept.h
funcPlayerVal.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcPlayerVal.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
funcPlayerVal.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h
funcPlayerVal.obj: dll.h funcPlayerVal.h Bexcept.h
funcRead.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcRead.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Instance.h
funcRead.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h Chunk.h
funcRead.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h
funcRead.obj: RefAction.h fileLIN.h filePBN.h fileRBN.h fileTXT.h Buffer.h
funcRead.obj: fileEML.h fileREC.h funcRead.h OrderCounts.h parse.h Bexcept.h
funcRefStats.obj: Buffer.h bconst.h RefLines.h RefLine.h RefEdit.h refconst.h
funcRefStats.obj: RefComment.h RefAction.h RefStats.h funcRefStats.h
funcRefStats.obj: Bexcept.h
funcTextStats.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcTextStats.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h
funcTextStats.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h
funcTextStats.obj: dll.h TextStats.h funcTextStats.h Bexcept.h
funcValidate.obj: validate.h bconst.h funcValidate.h ValStats.h ValProfile.h
funcValidate.obj: Bexcept.h
funcWrite.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcWrite.obj: Scoring.h Teams.h Team.h Board.h Deal.h Tableau.h Instance.h
funcWrite.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h fileLIN.h
funcWrite.obj: filePBN.h fileRBN.h fileTXT.h Buffer.h Chunk.h fileEML.h
funcWrite.obj: fileREC.h parse.h Bexcept.h
parse.obj: parse.h bconst.h
OrderCounts.obj: OrderCounts.h bconst.h
RefAction.obj: RefAction.h refconst.h Bexcept.h
RefComment.obj: RefComment.h refconst.h bconst.h parse.h Bexcept.h
RefEdit.obj: RefEdit.h refconst.h parse.h bconst.h Bexcept.h
RefLine.obj: RefLine.h RefEdit.h refconst.h RefComment.h bconst.h RefAction.h
RefLine.obj: parse.h Bexcept.h
RefLines.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
RefLines.obj: RefAction.h parse.h Bexcept.h
RefStats.obj: RefStats.h refconst.h bconst.h RefComment.h Bexcept.h
validate.obj: Chunk.h bconst.h ValStats.h ValProfile.h valint.h Buffer.h
validate.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h
validate.obj: RefAction.h filePBN.h validateLIN.h validatePBN.h validateRBN.h
validate.obj: validateTXT.h validateEML.h validateREC.h parse.h
validateLIN.obj: Deal.h bconst.h valint.h Buffer.h ValProfile.h validate.h
validateLIN.obj: validateLIN.h parse.h Bexcept.h Bdiff.h
validatePBN.obj: parse.h bconst.h validate.h valint.h Buffer.h ValProfile.h
validatePBN.obj: validatePBN.h Chunk.h
validateRBN.obj: validate.h bconst.h valint.h Buffer.h ValProfile.h
validateRBN.obj: validateRBN.h parse.h
validateTXT.obj: validate.h bconst.h valint.h Buffer.h ValProfile.h
validateTXT.obj: validateTXT.h parse.h
validateEML.obj: validate.h bconst.h valint.h Buffer.h ValProfile.h
validateEML.obj: validateEML.h
validateREC.obj: validate.h bconst.h valint.h Buffer.h ValProfile.h
validateREC.obj: validateREC.h parse.h
reader.obj: args.h bconst.h Files.h AllStats.h ValStats.h ValProfile.h
reader.obj: TextStats.h CompStats.h RefStats.h refconst.h Timers.h Timer.h
reader.obj: dispatch.h
reader.obj: args.h bconst.h Files.h AllStats.h ValStats.h ValProfile.h
reader.obj: TextStats.h CompStats.h RefStats.h refconst.h Timers.h Timer.h
reader.obj: dispatch.h
