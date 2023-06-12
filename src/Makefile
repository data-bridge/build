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
	HeaderLIN.cpp		\
	Instance.cpp		\
        Location.cpp            \
	OrderCounts.cpp		\
	Play.cpp		\
	Players.cpp		\
	RefAction.cpp		\
	RefComment.cpp		\
	RefEdit.cpp		\
	Refline.cpp		\
	Reflines.cpp		\
	RefStats.cpp		\
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
	funcEquals.cpp		\
	funcIMPSheet.cpp	\
	funcPlayerVal.cpp	\
	funcRead.cpp		\
	funcRefStats.cpp	\
	funcTextStats.cpp	\
	funcValidate.cpp	\
	funcValuation.cpp	\
	funcWrite.cpp		\
	parse.cpp		\
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
        /wd5045                 \
        /WX

CC_FLAGS_VS     = /O2 /Oi /Ot /Oy /GL /Zi /EHsc /std:c++17
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
Board.obj: bconst.h Board.h Deal.h Tableau.h GivenScore.h Instance.h
Board.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h parse.h
Board.obj: Bexcept.h Bdiff.h
Buffer.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
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
Group.obj: Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
Group.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
Group.obj: Bdiff.h
HeaderLIN.obj: HeaderLIN.h bconst.h parse.h Bexcept.h
Instance.obj: Instance.h Players.h bconst.h Auction.h Contract.h Play.h
Instance.obj: ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Location.obj: Location.h bconst.h Bexcept.h Bdiff.h
OrderCounts.obj: OrderCounts.h bconst.h
Play.obj: Play.h bconst.h ddsIF.h dll.h Contract.h parse.h Bexcept.h Bdiff.h
Players.obj: Players.h bconst.h parse.h Bexcept.h Bdiff.h
RefAction.obj: RefAction.h refconst.h Bexcept.h
RefComment.obj: RefComment.h refconst.h bconst.h parse.h Bexcept.h
RefEdit.obj: RefEdit.h refconst.h parse.h bconst.h Bexcept.h
RefLine.obj: RefLine.h RefEdit.h refconst.h RefComment.h bconst.h RefAction.h
RefLine.obj: parse.h Bexcept.h
RefLines.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
RefLines.obj: RefAction.h parse.h Bexcept.h
RefStats.obj: RefStats.h refconst.h bconst.h RefComment.h Bexcept.h
Scoring.obj: Scoring.h bconst.h Bexcept.h Bdiff.h
Segment.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
Segment.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
Segment.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
Segment.obj: parse.h Bexcept.h Bdiff.h
Session.obj: Session.h bconst.h parse.h Bdiff.h Bexcept.h
Sheet.obj: Buffer.h bconst.h Segment.h Date.h Location.h Session.h Scoring.h
Sheet.obj: Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
Sheet.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
Sheet.obj: Sheet.h SheetHand.h Reflines.h RefLine.h RefEdit.h refconst.h
Sheet.obj: RefComment.h RefAction.h parse.h Bexcept.h Bdiff.h
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
args.obj: args.h bconst.h
ddsIF.obj: ddsIF.h dll.h Bexcept.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
dispatch.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
dispatch.obj: ddsIF.h dll.h dispatch.h validate.h Files.h AllStats.h
dispatch.obj: ValStats.h ValProfile.h TextStats.h CompStats.h RefStats.h
dispatch.obj: refconst.h Timers.h Timer.h RefLines.h RefLine.h RefEdit.h
dispatch.obj: RefComment.h RefAction.h funcCompare.h funcDigest.h
dispatch.obj: funcEquals.h funcIMPSheet.h funcRead.h funcPlayerVal.h
dispatch.obj: funcRefStats.h funcTextStats.h funcValidate.h funcValuation.h
dispatch.obj: funcWrite.h
fileEML.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileEML.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
fileEML.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
fileEML.obj: Buffer.h Chunk.h Canvas.h fileEML.h parse.h Bexcept.h
filePBN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
filePBN.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
filePBN.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
filePBN.obj: Buffer.h Chunk.h filePBN.h Bexcept.h
fileRBN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileRBN.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
fileRBN.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
fileRBN.obj: Buffer.h Chunk.h fileRBN.h parse.h Bexcept.h
fileTXT.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileTXT.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
fileTXT.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
fileTXT.obj: Canvas.h fileTXT.h Buffer.h Chunk.h parse.h Bexcept.h
fileLIN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileLIN.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
fileLIN.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
fileLIN.obj: Buffer.h Chunk.h fileLIN.h parse.h Bexcept.h
fileREC.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileREC.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h GivenScore.h
fileREC.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
fileREC.obj: Buffer.h Chunk.h Canvas.h fileREC.h parse.h Bexcept.h
funcCompare.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcCompare.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcCompare.obj: Tableau.h GivenScore.h Instance.h Players.h Auction.h
funcCompare.obj: Contract.h Play.h ddsIF.h dll.h Buffer.h CompStats.h
funcCompare.obj: funcCompare.h funcRead.h Bexcept.h Bdiff.h
funcDigest.obj: funcDigest.h bconst.h Sheet.h Contract.h Deal.h Auction.h
funcDigest.obj: Play.h ddsIF.h dll.h Buffer.h SheetHand.h Reflines.h
funcDigest.obj: RefLine.h RefEdit.h refconst.h RefComment.h RefAction.h
funcDigest.obj: parse.h Bexcept.h
funcEquals.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcEquals.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcEquals.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
funcEquals.obj: ddsIF.h dll.h Bexcept.h
funcIMPSheet.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcIMPSheet.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcIMPSheet.obj: Tableau.h GivenScore.h Instance.h Players.h Auction.h
funcIMPSheet.obj: Contract.h Play.h ddsIF.h dll.h Sheet.h Buffer.h
funcIMPSheet.obj: SheetHand.h Reflines.h RefLine.h RefEdit.h refconst.h
funcIMPSheet.obj: RefComment.h RefAction.h funcIMPSheet.h Bexcept.h
funcPlayerVal.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcPlayerVal.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcPlayerVal.obj: Tableau.h GivenScore.h Instance.h Players.h Auction.h
funcPlayerVal.obj: Contract.h Play.h ddsIF.h dll.h funcPlayerVal.h Bexcept.h
funcRead.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcRead.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcRead.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
funcRead.obj: ddsIF.h dll.h Chunk.h RefLines.h RefLine.h RefEdit.h refconst.h
funcRead.obj: RefComment.h RefAction.h fileLIN.h filePBN.h fileRBN.h
funcRead.obj: fileTXT.h Buffer.h fileEML.h fileREC.h funcRead.h OrderCounts.h
funcRead.obj: parse.h Bexcept.h
funcRefStats.obj: Buffer.h bconst.h RefLines.h RefLine.h RefEdit.h refconst.h
funcRefStats.obj: RefComment.h RefAction.h RefStats.h funcRefStats.h
funcRefStats.obj: Bexcept.h
funcTextStats.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcTextStats.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcTextStats.obj: Tableau.h GivenScore.h Instance.h Players.h Auction.h
funcTextStats.obj: Contract.h Play.h ddsIF.h dll.h TextStats.h
funcTextStats.obj: funcTextStats.h Bexcept.h
funcValidate.obj: validate.h bconst.h funcValidate.h ValStats.h ValProfile.h
funcValidate.obj: Bexcept.h
funcValuation.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcValuation.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcValuation.obj: Tableau.h GivenScore.h Instance.h Players.h Auction.h
funcValuation.obj: Contract.h Play.h ddsIF.h dll.h Bexcept.h
funcWrite.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcWrite.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcWrite.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
funcWrite.obj: ddsIF.h dll.h fileLIN.h filePBN.h fileRBN.h fileTXT.h Buffer.h
funcWrite.obj: Chunk.h fileEML.h fileREC.h parse.h Bexcept.h
parse.obj: parse.h bconst.h
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
