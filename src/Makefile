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
	DDInfo.cpp		\
	Date.cpp		\
	Deal.cpp		\
	DuplStat.cpp		\
	DuplStats.cpp		\
	Files.cpp		\
	GivenScore.cpp		\
	Group.cpp		\
	HeaderLIN.cpp		\
	Instance.cpp		\
        Location.cpp            \
	OrderCounts.cpp		\
	Play.cpp		\
	Players.cpp		\
        PlayScore.cpp           \
        PlayTrace.cpp           \
	RefAction.cpp		\
	RefComment.cpp		\
	RefEdit.cpp		\
	RefLine.cpp		\
	RefLines.cpp		\
	RefStats.cpp		\
        Scoring.cpp             \
        Segment.cpp             \
        Session.cpp             \
	Sheet.cpp		\
	SheetHand.cpp		\
	Tableau.cpp		\
        Team.cpp                \
        Teams.cpp               \
	Term.cpp		\
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
	funcDD.cpp		\
	funcDigest.cpp		\
	funcDupl.cpp		\
	funcIMPSheet.cpp	\
	funcPlayerVal.cpp	\
	funcRead.cpp		\
	funcRefStats.cpp	\
	funcTextStats.cpp	\
	funcTrace.cpp		\
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
AllStats.obj: CompStats.h RefStats.h refconst.h DuplStats.h DuplStat.h
AllStats.obj: Timers.h Timer.h
Auction.obj: Auction.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Bdiff.obj: Bdiff.h
Bexcept.obj: Bexcept.h
Board.obj: bconst.h Board.h Deal.h Tableau.h Valuation.h Term.h GivenScore.h
Board.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
Board.obj: PlayTrace.h PlayScore.h parse.h Bexcept.h Bdiff.h
Buffer.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h bconst.h
Buffer.obj: RefAction.h Buffer.h parse.h Bexcept.h
Canvas.obj: Canvas.h
Chunk.obj: Chunk.h bconst.h parse.h Bexcept.h Bdiff.h
CompStats.obj: CompStats.h bconst.h parse.h
Contract.obj: Contract.h bconst.h parse.h Bexcept.h Bdiff.h
DDInfo.obj: DDInfo.h bconst.h parse.h Bexcept.h
Date.obj: Date.h bconst.h parse.h Bexcept.h Bdiff.h
Deal.obj: Deal.h bconst.h parse.h Bexcept.h Bdiff.h
DuplStat.obj: DuplStat.h bconst.h Group.h Segment.h Date.h Location.h
DuplStat.obj: Session.h Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
DuplStat.obj: Tableau.h Valuation.h Term.h GivenScore.h Instance.h Players.h
DuplStat.obj: Auction.h Contract.h Play.h ddsIF.h dll.h PlayTrace.h
DuplStat.obj: PlayScore.h RefLines.h RefLine.h RefEdit.h refconst.h
DuplStat.obj: RefComment.h RefAction.h parse.h Bexcept.h
DuplStats.obj: DuplStats.h DuplStat.h bconst.h Group.h Segment.h Date.h
DuplStats.obj: Location.h Session.h Scoring.h Teams.h Team.h HeaderLIN.h
DuplStats.obj: Board.h Deal.h Tableau.h Valuation.h Term.h GivenScore.h
DuplStats.obj: Instance.h Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
DuplStats.obj: PlayTrace.h PlayScore.h RefLines.h RefLine.h RefEdit.h
DuplStats.obj: refconst.h RefComment.h RefAction.h Bexcept.h
Files.obj: Files.h DDInfo.h bconst.h parse.h
GivenScore.obj: bconst.h GivenScore.h parse.h Bexcept.h Bdiff.h
Group.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
Group.obj: Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h
Group.obj: Term.h GivenScore.h Instance.h Players.h Auction.h Contract.h
Group.obj: Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h Bdiff.h
HeaderLIN.obj: HeaderLIN.h bconst.h parse.h Bexcept.h
Instance.obj: Instance.h Players.h bconst.h Auction.h Contract.h Play.h
Instance.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h parse.h Bexcept.h Bdiff.h
Location.obj: Location.h bconst.h Bexcept.h Bdiff.h
OrderCounts.obj: OrderCounts.h bconst.h
Play.obj: Play.h bconst.h ddsIF.h dll.h Contract.h parse.h Bexcept.h Bdiff.h
Players.obj: Players.h bconst.h parse.h Bexcept.h Bdiff.h
PlayScore.obj: Bexcept.h Bdiff.h
PlayTrace.obj: Bexcept.h Bdiff.h
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
Segment.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
Segment.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
Segment.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h parse.h Bexcept.h Bdiff.h
Session.obj: Session.h bconst.h parse.h Bdiff.h Bexcept.h
Sheet.obj: Buffer.h bconst.h Segment.h Date.h Location.h Session.h Scoring.h
Sheet.obj: Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h
Sheet.obj: Term.h GivenScore.h Instance.h Players.h Auction.h Contract.h
Sheet.obj: Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h Sheet.h SheetHand.h
Sheet.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h RefAction.h
Sheet.obj: parse.h Bexcept.h Bdiff.h
SheetHand.obj: SheetHand.h Contract.h bconst.h Deal.h Auction.h Play.h
SheetHand.obj: ddsIF.h dll.h parse.h Bexcept.h Bdiff.h
Tableau.obj: Tableau.h bconst.h Contract.h parse.h Bexcept.h Bdiff.h
Team.obj: Team.h bconst.h parse.h Bexcept.h Bdiff.h
Teams.obj: Teams.h Team.h bconst.h parse.h Bexcept.h Bdiff.h
Term.obj: Term.h Valuation.h bconst.h Bexcept.h
TextStats.obj: TextStats.h bconst.h parse.h
Timer.obj: Timer.h
Timers.obj: Timers.h Timer.h bconst.h
ValProfile.obj: ValProfile.h bconst.h
ValStats.obj: ValStats.h ValProfile.h bconst.h
Valuation.obj: Valuation.h Term.h bconst.h Bexcept.h
args.obj: args.h bconst.h
ddsIF.obj: ddsIF.h dll.h Bexcept.h
dispatch.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
dispatch.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
dispatch.obj: Valuation.h Term.h GivenScore.h Instance.h Players.h Auction.h
dispatch.obj: Contract.h Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h
dispatch.obj: dispatch.h validate.h Files.h DDInfo.h AllStats.h ValStats.h
dispatch.obj: ValProfile.h TextStats.h CompStats.h RefStats.h refconst.h
dispatch.obj: DuplStats.h DuplStat.h Timers.h Timer.h RefLines.h RefLine.h
dispatch.obj: RefEdit.h RefComment.h RefAction.h funcCompare.h funcDD.h
dispatch.obj: funcDigest.h funcDupl.h funcIMPSheet.h funcRead.h
dispatch.obj: funcPlayerVal.h funcRefStats.h funcTextStats.h funcTrace.h
dispatch.obj: funcValidate.h funcValuation.h funcWrite.h
fileEML.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileEML.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
fileEML.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
fileEML.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h Buffer.h Chunk.h Canvas.h
fileEML.obj: fileEML.h parse.h Bexcept.h
filePBN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
filePBN.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
filePBN.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
filePBN.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h Buffer.h Chunk.h filePBN.h
filePBN.obj: Bexcept.h
fileRBN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileRBN.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
fileRBN.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
fileRBN.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h Buffer.h Chunk.h fileRBN.h
fileRBN.obj: parse.h Bexcept.h
fileTXT.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileTXT.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
fileTXT.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
fileTXT.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h Canvas.h fileTXT.h
fileTXT.obj: Buffer.h Chunk.h parse.h Bexcept.h
fileLIN.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileLIN.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
fileLIN.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
fileLIN.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h Buffer.h Chunk.h fileLIN.h
fileLIN.obj: parse.h Bexcept.h
fileREC.obj: Segment.h Date.h bconst.h Location.h Session.h Scoring.h Teams.h
fileREC.obj: Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h Term.h
fileREC.obj: GivenScore.h Instance.h Players.h Auction.h Contract.h Play.h
fileREC.obj: ddsIF.h dll.h PlayTrace.h PlayScore.h Buffer.h Chunk.h Canvas.h
fileREC.obj: fileREC.h parse.h Bexcept.h
funcCompare.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcCompare.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcCompare.obj: Tableau.h Valuation.h Term.h GivenScore.h Instance.h
funcCompare.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcCompare.obj: PlayTrace.h PlayScore.h Buffer.h CompStats.h funcCompare.h
funcCompare.obj: funcRead.h Bexcept.h Bdiff.h
funcDD.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h Scoring.h
funcDD.obj: Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h Valuation.h
funcDD.obj: Term.h GivenScore.h Instance.h Players.h Auction.h Contract.h
funcDD.obj: Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h Files.h DDInfo.h
funcDD.obj: parse.h Bexcept.h
funcDigest.obj: funcDigest.h bconst.h Sheet.h Contract.h Deal.h Auction.h
funcDigest.obj: Play.h ddsIF.h dll.h Buffer.h SheetHand.h Reflines.h
funcDigest.obj: RefLine.h RefEdit.h refconst.h RefComment.h RefAction.h
funcDigest.obj: parse.h Bexcept.h
funcDupl.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcDupl.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcDupl.obj: Valuation.h Term.h GivenScore.h Instance.h Players.h Auction.h
funcDupl.obj: Contract.h Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h
funcDupl.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h
funcDupl.obj: RefAction.h DuplStats.h DuplStat.h Bexcept.h
funcIMPSheet.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcIMPSheet.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcIMPSheet.obj: Tableau.h Valuation.h Term.h GivenScore.h Instance.h
funcIMPSheet.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcIMPSheet.obj: PlayTrace.h PlayScore.h Sheet.h Buffer.h SheetHand.h
funcIMPSheet.obj: Reflines.h RefLine.h RefEdit.h refconst.h RefComment.h
funcIMPSheet.obj: RefAction.h funcIMPSheet.h Bexcept.h
funcPlayerVal.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcPlayerVal.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcPlayerVal.obj: Tableau.h Valuation.h Term.h GivenScore.h Instance.h
funcPlayerVal.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcPlayerVal.obj: PlayTrace.h PlayScore.h funcPlayerVal.h Bexcept.h
funcRead.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcRead.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcRead.obj: Valuation.h Term.h GivenScore.h Instance.h Players.h Auction.h
funcRead.obj: Contract.h Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h Chunk.h
funcRead.obj: RefLines.h RefLine.h RefEdit.h refconst.h RefComment.h
funcRead.obj: RefAction.h fileLIN.h filePBN.h fileRBN.h fileTXT.h Buffer.h
funcRead.obj: fileEML.h fileREC.h funcRead.h OrderCounts.h parse.h Bexcept.h
funcRefStats.obj: Buffer.h bconst.h RefLines.h RefLine.h RefEdit.h refconst.h
funcRefStats.obj: RefComment.h RefAction.h RefStats.h funcRefStats.h
funcRefStats.obj: Bexcept.h
funcTextStats.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcTextStats.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcTextStats.obj: Tableau.h Valuation.h Term.h GivenScore.h Instance.h
funcTextStats.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcTextStats.obj: PlayTrace.h PlayScore.h TextStats.h funcTextStats.h
funcTextStats.obj: Bexcept.h
funcTrace.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcTrace.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcTrace.obj: Valuation.h Term.h GivenScore.h Instance.h Players.h Auction.h
funcTrace.obj: Contract.h Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h
funcTrace.obj: Files.h DDInfo.h parse.h Bexcept.h
funcValidate.obj: validate.h bconst.h funcValidate.h ValStats.h ValProfile.h
funcValidate.obj: Bexcept.h
funcValuation.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcValuation.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h
funcValuation.obj: Tableau.h Valuation.h Term.h GivenScore.h Instance.h
funcValuation.obj: Players.h Auction.h Contract.h Play.h ddsIF.h dll.h
funcValuation.obj: PlayTrace.h PlayScore.h Bexcept.h
funcWrite.obj: Group.h Segment.h Date.h bconst.h Location.h Session.h
funcWrite.obj: Scoring.h Teams.h Team.h HeaderLIN.h Board.h Deal.h Tableau.h
funcWrite.obj: Valuation.h Term.h GivenScore.h Instance.h Players.h Auction.h
funcWrite.obj: Contract.h Play.h ddsIF.h dll.h PlayTrace.h PlayScore.h
funcWrite.obj: fileLIN.h filePBN.h fileRBN.h fileTXT.h Buffer.h Chunk.h
funcWrite.obj: fileEML.h fileREC.h parse.h Bexcept.h
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
reader.obj: args.h bconst.h Files.h DDInfo.h AllStats.h ValStats.h
reader.obj: ValProfile.h TextStats.h CompStats.h RefStats.h refconst.h
reader.obj: DuplStats.h DuplStat.h Timers.h Timer.h dispatch.h
reader.obj: args.h bconst.h Files.h DDInfo.h AllStats.h ValStats.h
reader.obj: ValProfile.h TextStats.h CompStats.h RefStats.h refconst.h
reader.obj: DuplStats.h DuplStat.h Timers.h Timer.h dispatch.h
