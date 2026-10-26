EXPORTER	= Exports.def

INCL_SOURCE	= Makefiles/sources
INCL_DEPENDS	= Makefiles/depends_obj

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
        /wd4464                 \
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
        /wd5039                 \
        /wd5045                 \
        /WX

CC_FLAGS_VS     = /O2 /Oi /Ot /Oy /Zi /EHsc /std:c++17
# CC_FLAGS_VS     = /O2 /Oi /Ot /Oy /GL /Zi /EHsc /std:c++17
# CC_FLAGS_VS     = /O2 /Oi /Ot /Oy /GL /Zi /EHa /DEBUG
CC_FLAGS_DEBUG  = /DEBUG


LD_FLAGS_VS	= /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF
# LD_FLAGS_VS	= /INCREMENTAL:NO /DEBUG

WINDRES_FLAG	= -F pe-i386

CC		= $(CC_VS)
CC_FLAGS	= $(CC_FLAGS_VS) $(WARN_FLAGS)
# CC_FLAGS	= $(CC_FLAGS_DEBUG) $(WARN_FLAGS) /Zi
LD_FLAGS	= $(LD_FLAGS_VS)

include $(INCL_SOURCE)

OBJ_FILES 	= $(subst .cpp,.obj,$(SOURCE_FILES))

# Linking directly.

reader:	$(OBJ_FILES)
	link $(LD_FLAGS) $(OBJ_FILES) $(DLIB) /out:$(TEST).exe

# link /LTCG $(LD_FLAGS) $(OBJ_FILES) $(DLIB) /out:$(TEST).exe

%.obj:	%.cpp
	$(CC) $(CC_FLAGS) /c $< -Fo$@


depend:
	makedepend -Y -o.obj -- $(CC_FLAGS) -- $(SOURCE_FILES) $(TEST).cpp

clean:
	rm -f $(OBJ_FILES) $(TEST_OBJ_FILES) $(TEST).obj $(TEST).exe 

include $(INCL_DEPENDS)

# DO NOT DELETE

reader.obj: control/args.h control/Options.h analysis/DistMatcher.h
reader.obj: analysis/DistElem.h files/Files.h files/FileTask.h
reader.obj: include/Format.h analysis/DDInfo.h dispatch/dispatch.h
reader.obj: dispatch/funcPasses.h stats/AllStats.h stats/Timer.h
analysis/DDInfo.obj: analysis/DDInfo.h util/parse.h include/bridge.h
analysis/DDInfo.obj: include/Format.h include/Label.h handling/Bexcept.h
analysis/DistElem.obj: analysis/DistElem.h util/parse.h include/bridge.h
analysis/DistElem.obj: include/Format.h include/Label.h handling/Bexcept.h
analysis/DistMatcher.obj: analysis/DistMatcher.h analysis/DistElem.h
analysis/DistMatcher.obj: util/parse.h include/bridge.h include/Format.h
analysis/DistMatcher.obj: include/Label.h handling/Bexcept.h
analysis/Distribution.obj: analysis/Distribution.h analysis/Distributions.h
analysis/Term.obj: analysis/Term.h analysis/Valuation.h analysis/Composites.h
analysis/Term.obj: include/bridge.h include/Format.h include/Label.h
analysis/Term.obj: handling/Bexcept.h
analysis/Valuation.obj: analysis/Valuation.h analysis/Term.h
analysis/Valuation.obj: analysis/Composites.h include/bridge.h
analysis/Valuation.obj: include/Format.h include/Label.h
analysis/Valuation.obj: analysis/DistMatcher.h analysis/DistElem.h
analysis/Valuation.obj: handling/Bexcept.h
analysis/ddsIF.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
analysis/ddsIF.obj: handling/Bexcept.h
analysis/passes/PassTerm.obj: analysis/Valuation.h analysis/Term.h
analysis/passes/PassTerm.obj: analysis/Composites.h include/bridge.h
analysis/passes/PassTerm.obj: include/Format.h include/Label.h
analysis/passes/PassTerm.obj: analysis/Composites.h
analysis/passes/PassTerm.obj: analysis/passes/PassTerm.h
analysis/passes/PassRow.obj: analysis/Valuation.h analysis/Term.h
analysis/passes/PassRow.obj: analysis/Composites.h include/bridge.h
analysis/passes/PassRow.obj: include/Format.h include/Label.h stats/RowData.h
analysis/passes/PassRow.obj: analysis/passes/Sigmoid.h
analysis/passes/PassRow.obj: analysis/passes/RowProbInfo.h
analysis/passes/PassRow.obj: analysis/passes/SigmoidData.h
analysis/passes/PassRow.obj: analysis/passes/PassRow.h
analysis/passes/PassRow.obj: analysis/passes/PassTerm.h
analysis/passes/PassTable.obj: analysis/Composites.h stats/RowData.h
analysis/passes/PassTable.obj: util/parse.h include/bridge.h include/Format.h
analysis/passes/PassTable.obj: include/Label.h analysis/passes/Sigmoid.h
analysis/passes/PassTable.obj: analysis/passes/RowProbInfo.h
analysis/passes/PassTable.obj: analysis/passes/SigmoidData.h
analysis/passes/PassTable.obj: analysis/passes/PassTable.h
analysis/passes/PassTable.obj: analysis/passes/PassRow.h
analysis/passes/PassTable.obj: analysis/passes/PassTerm.h
analysis/passes/PassTable.obj: analysis/Valuation.h analysis/Term.h
analysis/passes/PassTable.obj: analysis/Composites.h
analysis/passes/PassTables.obj: analysis/Valuation.h analysis/Term.h
analysis/passes/PassTables.obj: analysis/Composites.h include/bridge.h
analysis/passes/PassTables.obj: include/Format.h include/Label.h
analysis/passes/PassTables.obj: analysis/Distribution.h
analysis/passes/PassTables.obj: analysis/Distributions.h stats/AllStats.h
analysis/passes/PassTables.obj: stats/RuleStats.h stats/Rules.h stats/Rule.h
analysis/passes/PassTables.obj: include/bridge.h util/parse.h
analysis/passes/PassTables.obj: analysis/passes/PassTables.h
analysis/passes/PassTables.obj: analysis/passes/PassTable.h
analysis/passes/PassTables.obj: analysis/passes/PassRow.h
analysis/passes/PassTables.obj: analysis/passes/PassTerm.h
analysis/passes/PassTables.obj: analysis/passes/SigmoidData.h
analysis/passes/Sigmoid.obj: analysis/passes/SigmoidData.h
analysis/passes/Sigmoid.obj: analysis/passes/Sigmoid.h
control/args.obj: control/args.h control/Options.h analysis/DistMatcher.h
control/args.obj: analysis/DistElem.h include/Format.h
dispatch/OrderCounts.obj: dispatch/OrderCounts.h dispatch/Order.h
dispatch/dispatch.obj: dispatch/dispatch.h control/Options.h
dispatch/dispatch.obj: analysis/DistMatcher.h analysis/DistElem.h
dispatch/dispatch.obj: records/Group.h records/Segment.h records/Date.h
dispatch/dispatch.obj: records/Location.h records/Session.h records/Teams.h
dispatch/dispatch.obj: records/Team.h records/Scoring.h records/HeaderLIN.h
dispatch/dispatch.obj: records/LINData.h records/Board.h records/Deal.h
dispatch/dispatch.obj: include/bridge.h include/Format.h include/Label.h
dispatch/dispatch.obj: records/Tableau.h records/LeadTableau.h
dispatch/dispatch.obj: analysis/LeadTriple.h records/Instance.h
dispatch/dispatch.obj: records/Players.h records/Auction.h records/Contract.h
dispatch/dispatch.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
dispatch/dispatch.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/dispatch.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/dispatch.obj: records/GivenScore.h analysis/Valuation.h
dispatch/dispatch.obj: analysis/Term.h analysis/Composites.h files/Files.h
dispatch/dispatch.obj: files/FileTask.h include/Format.h analysis/DDInfo.h
dispatch/dispatch.obj: edits/RefLines.h edits/RefLine.h edits/RefEdit.h
dispatch/dispatch.obj: edits/RefCount.h edits/refconst.h edits/RefComment.h
dispatch/dispatch.obj: edits/RefEntry.h edits/RefAction.h stats/AllStats.h
dispatch/dispatch.obj: validate/validate.h analysis/passes/PassTable.h
dispatch/dispatch.obj: analysis/passes/PassRow.h analysis/passes/PassTerm.h
dispatch/dispatch.obj: analysis/passes/SigmoidData.h dispatch/funcClear.h
dispatch/dispatch.obj: dispatch/funcCompare.h dispatch/funcDD.h
dispatch/dispatch.obj: dispatch/funcDigest.h dispatch/funcDupl.h
dispatch/dispatch.obj: dispatch/funcIMPSheet.h dispatch/funcLeads.h
dispatch/dispatch.obj: dispatch/funcPasses.h dispatch/funcRead.h
dispatch/dispatch.obj: dispatch/funcPlayerVal.h dispatch/funcRefStats.h
dispatch/dispatch.obj: dispatch/funcTextStats.h dispatch/funcTrace.h
dispatch/dispatch.obj: dispatch/funcValidate.h dispatch/funcValuation.h
dispatch/dispatch.obj: dispatch/funcWrite.h stats/Timers.h stats/Timer.h
dispatch/funcClear.obj: files/Files.h files/FileTask.h include/Format.h
dispatch/funcClear.obj: analysis/DDInfo.h records/Group.h records/Segment.h
dispatch/funcClear.obj: records/Date.h records/Location.h records/Session.h
dispatch/funcClear.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcClear.obj: records/HeaderLIN.h records/LINData.h records/Board.h
dispatch/funcClear.obj: records/Deal.h include/bridge.h include/Format.h
dispatch/funcClear.obj: include/Label.h records/Tableau.h
dispatch/funcClear.obj: records/LeadTableau.h analysis/LeadTriple.h
dispatch/funcClear.obj: records/Instance.h records/Players.h
dispatch/funcClear.obj: records/Auction.h records/Contract.h records/Play.h
dispatch/funcClear.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcClear.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcClear.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcClear.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcClear.obj: analysis/Term.h analysis/Composites.h analysis/dll.h
dispatch/funcClear.obj: util/parse.h handling/Bexcept.h
dispatch/funcCompare.obj: dispatch/funcCompare.h dispatch/funcRead.h
dispatch/funcCompare.obj: dispatch/Order.h files/Buffer.h files/LineData.h
dispatch/funcCompare.obj: records/Group.h records/Segment.h records/Date.h
dispatch/funcCompare.obj: records/Location.h records/Session.h
dispatch/funcCompare.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcCompare.obj: records/HeaderLIN.h records/LINData.h
dispatch/funcCompare.obj: records/Board.h records/Deal.h include/bridge.h
dispatch/funcCompare.obj: include/Format.h include/Label.h records/Tableau.h
dispatch/funcCompare.obj: records/LeadTableau.h analysis/LeadTriple.h
dispatch/funcCompare.obj: records/Instance.h records/Players.h
dispatch/funcCompare.obj: records/Auction.h records/Contract.h records/Play.h
dispatch/funcCompare.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcCompare.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcCompare.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcCompare.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcCompare.obj: analysis/Term.h analysis/Composites.h
dispatch/funcCompare.obj: stats/CompStats.h include/Format.h
dispatch/funcCompare.obj: handling/Bexcept.h handling/Bdiff.h
dispatch/funcDD.obj: files/Files.h files/FileTask.h include/Format.h
dispatch/funcDD.obj: analysis/DDInfo.h records/Group.h records/Segment.h
dispatch/funcDD.obj: records/Date.h records/Location.h records/Session.h
dispatch/funcDD.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcDD.obj: records/HeaderLIN.h records/LINData.h records/Board.h
dispatch/funcDD.obj: records/Deal.h include/bridge.h include/Format.h
dispatch/funcDD.obj: include/Label.h records/Tableau.h records/LeadTableau.h
dispatch/funcDD.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcDD.obj: records/Players.h records/Auction.h records/Contract.h
dispatch/funcDD.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcDD.obj: analysis/LeadTriple.h analysis/dll.h records/PlayTrace.h
dispatch/funcDD.obj: analysis/PlayError.h records/GivenScore.h
dispatch/funcDD.obj: analysis/Valuation.h analysis/Term.h
dispatch/funcDD.obj: analysis/Composites.h util/parse.h analysis/dll.h
dispatch/funcDD.obj: handling/Bexcept.h
dispatch/funcDigest.obj: dispatch/funcDigest.h control/Options.h
dispatch/funcDigest.obj: analysis/DistMatcher.h analysis/DistElem.h
dispatch/funcDigest.obj: files/FileTask.h records/Sheet.h records/Contract.h
dispatch/funcDigest.obj: include/bridge.h include/Format.h include/Label.h
dispatch/funcDigest.obj: records/Deal.h records/Auction.h records/Play.h
dispatch/funcDigest.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcDigest.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcDigest.obj: records/SheetHand.h edits/Reflines.h edits/RefLine.h
dispatch/funcDigest.obj: edits/RefEdit.h edits/RefCount.h edits/refconst.h
dispatch/funcDigest.obj: include/Format.h edits/RefComment.h edits/RefEntry.h
dispatch/funcDigest.obj: edits/RefAction.h util/parse.h handling/Bexcept.h
dispatch/funcDupl.obj: records/Group.h records/Segment.h records/Date.h
dispatch/funcDupl.obj: records/Location.h records/Session.h records/Teams.h
dispatch/funcDupl.obj: records/Team.h records/Scoring.h records/HeaderLIN.h
dispatch/funcDupl.obj: records/LINData.h records/Board.h records/Deal.h
dispatch/funcDupl.obj: include/bridge.h include/Format.h include/Label.h
dispatch/funcDupl.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcDupl.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcDupl.obj: records/Players.h records/Auction.h records/Contract.h
dispatch/funcDupl.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcDupl.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcDupl.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcDupl.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcDupl.obj: analysis/Term.h analysis/Composites.h edits/RefLines.h
dispatch/funcDupl.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
dispatch/funcDupl.obj: edits/refconst.h include/Format.h edits/RefComment.h
dispatch/funcDupl.obj: edits/RefEntry.h edits/RefAction.h stats/DuplStats.h
dispatch/funcDupl.obj: stats/DuplStat.h handling/Bexcept.h
dispatch/funcEquals.obj: records/Group.h records/Segment.h records/Date.h
dispatch/funcEquals.obj: records/Location.h records/Session.h records/Teams.h
dispatch/funcEquals.obj: records/Team.h records/Scoring.h records/HeaderLIN.h
dispatch/funcEquals.obj: records/LINData.h records/Board.h records/Deal.h
dispatch/funcEquals.obj: include/bridge.h include/Format.h include/Label.h
dispatch/funcEquals.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcEquals.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcEquals.obj: records/Players.h records/Auction.h
dispatch/funcEquals.obj: records/Contract.h records/Play.h
dispatch/funcEquals.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcEquals.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcEquals.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcEquals.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcEquals.obj: analysis/Term.h analysis/Composites.h
dispatch/funcEquals.obj: handling/Bexcept.h
dispatch/funcIMPSheet.obj: dispatch/funcIMPSheet.h records/Group.h
dispatch/funcIMPSheet.obj: records/Segment.h records/Date.h
dispatch/funcIMPSheet.obj: records/Location.h records/Session.h
dispatch/funcIMPSheet.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcIMPSheet.obj: records/HeaderLIN.h records/LINData.h
dispatch/funcIMPSheet.obj: records/Board.h records/Deal.h include/bridge.h
dispatch/funcIMPSheet.obj: include/Format.h include/Label.h records/Tableau.h
dispatch/funcIMPSheet.obj: records/LeadTableau.h analysis/LeadTriple.h
dispatch/funcIMPSheet.obj: records/Instance.h records/Players.h
dispatch/funcIMPSheet.obj: records/Auction.h records/Contract.h
dispatch/funcIMPSheet.obj: records/Play.h records/SDKnowledge.h
dispatch/funcIMPSheet.obj: analysis/ddsIF.h analysis/LeadTriple.h
dispatch/funcIMPSheet.obj: analysis/dll.h records/PlayTrace.h
dispatch/funcIMPSheet.obj: analysis/PlayError.h records/GivenScore.h
dispatch/funcIMPSheet.obj: analysis/Valuation.h analysis/Term.h
dispatch/funcIMPSheet.obj: analysis/Composites.h records/Sheet.h
dispatch/funcIMPSheet.obj: records/SheetHand.h edits/Reflines.h
dispatch/funcIMPSheet.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
dispatch/funcIMPSheet.obj: edits/refconst.h include/Format.h
dispatch/funcIMPSheet.obj: edits/RefComment.h edits/RefEntry.h
dispatch/funcIMPSheet.obj: edits/RefAction.h handling/Bexcept.h
dispatch/funcLeads.obj: files/Files.h files/FileTask.h include/Format.h
dispatch/funcLeads.obj: analysis/DDInfo.h records/Group.h records/Segment.h
dispatch/funcLeads.obj: records/Date.h records/Location.h records/Session.h
dispatch/funcLeads.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcLeads.obj: records/HeaderLIN.h records/LINData.h records/Board.h
dispatch/funcLeads.obj: records/Deal.h include/bridge.h include/Format.h
dispatch/funcLeads.obj: include/Label.h records/Tableau.h
dispatch/funcLeads.obj: records/LeadTableau.h analysis/LeadTriple.h
dispatch/funcLeads.obj: records/Instance.h records/Players.h
dispatch/funcLeads.obj: records/Auction.h records/Contract.h records/Play.h
dispatch/funcLeads.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcLeads.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcLeads.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcLeads.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcLeads.obj: analysis/Term.h analysis/Composites.h util/parse.h
dispatch/funcLeads.obj: analysis/dll.h handling/Bexcept.h
dispatch/funcPasses.obj: control/Options.h analysis/DistMatcher.h
dispatch/funcPasses.obj: analysis/DistElem.h records/Group.h
dispatch/funcPasses.obj: records/Segment.h records/Date.h records/Location.h
dispatch/funcPasses.obj: records/Session.h records/Teams.h records/Team.h
dispatch/funcPasses.obj: records/Scoring.h records/HeaderLIN.h
dispatch/funcPasses.obj: records/LINData.h records/Board.h records/Deal.h
dispatch/funcPasses.obj: include/bridge.h include/Format.h include/Label.h
dispatch/funcPasses.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcPasses.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcPasses.obj: records/Players.h records/Auction.h
dispatch/funcPasses.obj: records/Contract.h records/Play.h
dispatch/funcPasses.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcPasses.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcPasses.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcPasses.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcPasses.obj: analysis/Term.h analysis/Composites.h
dispatch/funcPasses.obj: records/Sheet.h records/SheetHand.h edits/Reflines.h
dispatch/funcPasses.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
dispatch/funcPasses.obj: edits/refconst.h include/Format.h edits/RefComment.h
dispatch/funcPasses.obj: edits/RefEntry.h edits/RefAction.h
dispatch/funcPasses.obj: analysis/Distribution.h analysis/Distributions.h
dispatch/funcPasses.obj: dispatch/funcPasses.h dispatch/Opening.h
dispatch/funcPasses.obj: dispatch/Openings.h util/parse.h edits/RefLines.h
dispatch/funcPasses.obj: stats/ParamStats1D.h stats/Stats1D.h
dispatch/funcPasses.obj: stats/StatsInfo.h stats/ParamStats2D.h
dispatch/funcPasses.obj: stats/Stats2D.h stats/RuleStats.h stats/Rules.h
dispatch/funcPasses.obj: stats/Rule.h handling/Bexcept.h
dispatch/funcPasses.obj: analysis/passes/PassTables.h
dispatch/funcPasses.obj: analysis/passes/PassTable.h
dispatch/funcPasses.obj: analysis/passes/PassRow.h analysis/passes/PassTerm.h
dispatch/funcPasses.obj: analysis/passes/SigmoidData.h
dispatch/funcPlayerVal.obj: records/Group.h records/Segment.h records/Date.h
dispatch/funcPlayerVal.obj: records/Location.h records/Session.h
dispatch/funcPlayerVal.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcPlayerVal.obj: records/HeaderLIN.h records/LINData.h
dispatch/funcPlayerVal.obj: records/Board.h records/Deal.h include/bridge.h
dispatch/funcPlayerVal.obj: include/Format.h include/Label.h
dispatch/funcPlayerVal.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcPlayerVal.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcPlayerVal.obj: records/Players.h records/Auction.h
dispatch/funcPlayerVal.obj: records/Contract.h records/Play.h
dispatch/funcPlayerVal.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcPlayerVal.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcPlayerVal.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcPlayerVal.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcPlayerVal.obj: analysis/Term.h analysis/Composites.h
dispatch/funcPlayerVal.obj: dispatch/funcPlayerVal.h handling/Bexcept.h
dispatch/funcRead.obj: dispatch/funcRead.h dispatch/OrderCounts.h
dispatch/funcRead.obj: dispatch/Order.h formats/fileLIN.h formats/filePBN.h
dispatch/funcRead.obj: formats/fileRBN.h formats/fileTXT.h formats/fileEML.h
dispatch/funcRead.obj: formats/fileREC.h control/Options.h
dispatch/funcRead.obj: analysis/DistMatcher.h analysis/DistElem.h
dispatch/funcRead.obj: files/Buffer.h files/LineData.h files/Chunk.h
dispatch/funcRead.obj: dispatch/Order.h include/Label.h records/Group.h
dispatch/funcRead.obj: records/Segment.h records/Date.h records/Location.h
dispatch/funcRead.obj: records/Session.h records/Teams.h records/Team.h
dispatch/funcRead.obj: records/Scoring.h records/HeaderLIN.h
dispatch/funcRead.obj: records/LINData.h records/Board.h records/Deal.h
dispatch/funcRead.obj: include/bridge.h include/Format.h include/Label.h
dispatch/funcRead.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcRead.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcRead.obj: records/Players.h records/Auction.h records/Contract.h
dispatch/funcRead.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcRead.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcRead.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcRead.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcRead.obj: analysis/Term.h analysis/Composites.h edits/RefLines.h
dispatch/funcRead.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
dispatch/funcRead.obj: edits/refconst.h include/Format.h edits/RefComment.h
dispatch/funcRead.obj: edits/RefEntry.h edits/RefAction.h util/parse.h
dispatch/funcRead.obj: handling/Bexcept.h
dispatch/funcRefStats.obj: dispatch/funcRefStats.h files/Buffer.h
dispatch/funcRefStats.obj: files/LineData.h edits/RefLines.h edits/RefLine.h
dispatch/funcRefStats.obj: edits/RefEdit.h edits/RefCount.h edits/refconst.h
dispatch/funcRefStats.obj: include/Format.h edits/RefComment.h
dispatch/funcRefStats.obj: edits/RefEntry.h edits/RefAction.h edits/Comment.h
dispatch/funcRefStats.obj: stats/RefStats.h edits/RefEntry.h
dispatch/funcRefStats.obj: handling/Bexcept.h
dispatch/funcTextStats.obj: dispatch/funcTextStats.h records/Group.h
dispatch/funcTextStats.obj: records/Segment.h records/Date.h
dispatch/funcTextStats.obj: records/Location.h records/Session.h
dispatch/funcTextStats.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcTextStats.obj: records/HeaderLIN.h records/LINData.h
dispatch/funcTextStats.obj: records/Board.h records/Deal.h include/bridge.h
dispatch/funcTextStats.obj: include/Format.h include/Label.h
dispatch/funcTextStats.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcTextStats.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcTextStats.obj: records/Players.h records/Auction.h
dispatch/funcTextStats.obj: records/Contract.h records/Play.h
dispatch/funcTextStats.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcTextStats.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcTextStats.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcTextStats.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcTextStats.obj: analysis/Term.h analysis/Composites.h
dispatch/funcTextStats.obj: files/FileTask.h stats/TextStats.h
dispatch/funcTextStats.obj: stats/TextStat.h stats/TextDatum.h
dispatch/funcTextStats.obj: handling/Bexcept.h
dispatch/funcTrace.obj: files/Files.h files/FileTask.h include/Format.h
dispatch/funcTrace.obj: analysis/DDInfo.h records/Group.h records/Segment.h
dispatch/funcTrace.obj: records/Date.h records/Location.h records/Session.h
dispatch/funcTrace.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcTrace.obj: records/HeaderLIN.h records/LINData.h records/Board.h
dispatch/funcTrace.obj: records/Deal.h include/bridge.h include/Format.h
dispatch/funcTrace.obj: include/Label.h records/Tableau.h
dispatch/funcTrace.obj: records/LeadTableau.h analysis/LeadTriple.h
dispatch/funcTrace.obj: records/Instance.h records/Players.h
dispatch/funcTrace.obj: records/Auction.h records/Contract.h records/Play.h
dispatch/funcTrace.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcTrace.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcTrace.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcTrace.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcTrace.obj: analysis/Term.h analysis/Composites.h analysis/dll.h
dispatch/funcTrace.obj: util/parse.h handling/Bexcept.h
dispatch/funcValidate.obj: dispatch/funcValidate.h validate/validate.h
dispatch/funcValidate.obj: files/FileTask.h stats/ValStats.h stats/ValRow.h
dispatch/funcValidate.obj: validate/ValProfile.h include/Format.h
dispatch/funcValidate.obj: include/Label.h handling/Bexcept.h
dispatch/funcValuation.obj: records/Group.h records/Segment.h records/Date.h
dispatch/funcValuation.obj: records/Location.h records/Session.h
dispatch/funcValuation.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcValuation.obj: records/HeaderLIN.h records/LINData.h
dispatch/funcValuation.obj: records/Board.h records/Deal.h include/bridge.h
dispatch/funcValuation.obj: include/Format.h include/Label.h
dispatch/funcValuation.obj: records/Tableau.h records/LeadTableau.h
dispatch/funcValuation.obj: analysis/LeadTriple.h records/Instance.h
dispatch/funcValuation.obj: records/Players.h records/Auction.h
dispatch/funcValuation.obj: records/Contract.h records/Play.h
dispatch/funcValuation.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcValuation.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcValuation.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcValuation.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcValuation.obj: analysis/Term.h analysis/Composites.h
dispatch/funcValuation.obj: handling/Bexcept.h
dispatch/funcWrite.obj: dispatch/Order.h records/Group.h records/Segment.h
dispatch/funcWrite.obj: records/Date.h records/Location.h records/Session.h
dispatch/funcWrite.obj: records/Teams.h records/Team.h records/Scoring.h
dispatch/funcWrite.obj: records/HeaderLIN.h records/LINData.h records/Board.h
dispatch/funcWrite.obj: records/Deal.h include/bridge.h include/Format.h
dispatch/funcWrite.obj: include/Label.h records/Tableau.h
dispatch/funcWrite.obj: records/LeadTableau.h analysis/LeadTriple.h
dispatch/funcWrite.obj: records/Instance.h records/Players.h
dispatch/funcWrite.obj: records/Auction.h records/Contract.h records/Play.h
dispatch/funcWrite.obj: records/SDKnowledge.h analysis/ddsIF.h
dispatch/funcWrite.obj: analysis/LeadTriple.h analysis/dll.h
dispatch/funcWrite.obj: records/PlayTrace.h analysis/PlayError.h
dispatch/funcWrite.obj: records/GivenScore.h analysis/Valuation.h
dispatch/funcWrite.obj: analysis/Term.h analysis/Composites.h
dispatch/funcWrite.obj: formats/fileLIN.h formats/filePBN.h formats/fileRBN.h
dispatch/funcWrite.obj: formats/fileTXT.h formats/fileEML.h formats/fileREC.h
dispatch/funcWrite.obj: formats/writeInfo.h util/parse.h handling/Bexcept.h
dispatch/Opening.obj: dispatch/Opening.h dispatch/Openings.h
dispatch/Opening.obj: analysis/Valuation.h analysis/Term.h
dispatch/Opening.obj: analysis/Composites.h include/bridge.h include/Format.h
dispatch/Opening.obj: include/Label.h
edits/RefAction.obj: edits/RefAction.h edits/refconst.h include/Format.h
edits/RefAction.obj: handling/Bexcept.h
edits/RefComment.obj: edits/RefComment.h edits/RefEntry.h edits/RefCount.h
edits/RefComment.obj: edits/refconst.h include/Format.h edits/Comment.h
edits/RefComment.obj: util/parse.h include/bridge.h include/Format.h
edits/RefComment.obj: include/Label.h handling/Bexcept.h
edits/RefCount.obj: edits/RefCount.h
edits/RefEdit.obj: edits/RefEdit.h edits/RefCount.h edits/refconst.h
edits/RefEdit.obj: include/Format.h util/parse.h include/bridge.h
edits/RefEdit.obj: include/Format.h include/Label.h handling/Bexcept.h
edits/RefEntry.obj: edits/RefEntry.h edits/RefCount.h
edits/Refline.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
edits/Refline.obj: edits/refconst.h include/Format.h edits/RefComment.h
edits/Refline.obj: edits/RefEntry.h edits/RefAction.h edits/Comment.h
edits/Refline.obj: util/parse.h include/bridge.h include/Format.h
edits/Refline.obj: include/Label.h handling/Bexcept.h
edits/Reflines.obj: edits/RefLines.h edits/RefLine.h edits/RefEdit.h
edits/Reflines.obj: edits/RefCount.h edits/refconst.h include/Format.h
edits/Reflines.obj: edits/RefComment.h edits/RefEntry.h edits/RefAction.h
edits/Reflines.obj: edits/Comment.h dispatch/Order.h files/Buffer.h
edits/Reflines.obj: files/LineData.h util/parse.h include/bridge.h
edits/Reflines.obj: include/Format.h include/Label.h handling/Bexcept.h
files/Buffer.obj: files/Buffer.h files/LineData.h edits/RefLines.h
files/Buffer.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
files/Buffer.obj: edits/refconst.h include/Format.h edits/RefComment.h
files/Buffer.obj: edits/RefEntry.h edits/RefAction.h util/parse.h
files/Buffer.obj: include/bridge.h include/Format.h include/Label.h
files/Buffer.obj: handling/Bexcept.h
files/Chunk.obj: files/Chunk.h dispatch/Order.h include/Label.h util/parse.h
files/Chunk.obj: include/bridge.h include/Format.h include/Label.h
files/Chunk.obj: handling/Bexcept.h handling/Bdiff.h
files/Files.obj: files/Files.h files/FileTask.h include/Format.h
files/Files.obj: analysis/DDInfo.h util/parse.h include/bridge.h
files/Files.obj: include/Format.h include/Label.h control/Options.h
files/Files.obj: analysis/DistMatcher.h analysis/DistElem.h
files/FileTask.obj: files/FileTask.h
formats/Canvas.obj: formats/Canvas.h
formats/fileEML.obj: formats/fileEML.h formats/Canvas.h formats/WriteInfo.h
formats/fileEML.obj: records/Segment.h records/Date.h records/Location.h
formats/fileEML.obj: records/Session.h records/Teams.h records/Team.h
formats/fileEML.obj: records/Scoring.h records/HeaderLIN.h records/LINData.h
formats/fileEML.obj: records/Board.h records/Deal.h include/bridge.h
formats/fileEML.obj: include/Format.h include/Label.h records/Tableau.h
formats/fileEML.obj: records/LeadTableau.h analysis/LeadTriple.h
formats/fileEML.obj: records/Instance.h records/Players.h records/Auction.h
formats/fileEML.obj: records/Contract.h records/Play.h records/SDKnowledge.h
formats/fileEML.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
formats/fileEML.obj: records/PlayTrace.h analysis/PlayError.h
formats/fileEML.obj: records/GivenScore.h analysis/Valuation.h
formats/fileEML.obj: analysis/Term.h analysis/Composites.h records/Board.h
formats/fileEML.obj: files/Buffer.h files/LineData.h files/Chunk.h
formats/fileEML.obj: dispatch/Order.h include/Label.h util/parse.h
formats/fileEML.obj: handling/Bexcept.h
formats/fileLIN.obj: formats/fileLIN.h formats/WriteInfo.h records/Segment.h
formats/fileLIN.obj: records/Date.h records/Location.h records/Session.h
formats/fileLIN.obj: records/Teams.h records/Team.h records/Scoring.h
formats/fileLIN.obj: records/HeaderLIN.h records/LINData.h records/Board.h
formats/fileLIN.obj: records/Deal.h include/bridge.h include/Format.h
formats/fileLIN.obj: include/Label.h records/Tableau.h records/LeadTableau.h
formats/fileLIN.obj: analysis/LeadTriple.h records/Instance.h
formats/fileLIN.obj: records/Players.h records/Auction.h records/Contract.h
formats/fileLIN.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
formats/fileLIN.obj: analysis/LeadTriple.h analysis/dll.h records/PlayTrace.h
formats/fileLIN.obj: analysis/PlayError.h records/GivenScore.h
formats/fileLIN.obj: analysis/Valuation.h analysis/Term.h
formats/fileLIN.obj: analysis/Composites.h records/Board.h files/Buffer.h
formats/fileLIN.obj: files/LineData.h files/Chunk.h dispatch/Order.h
formats/fileLIN.obj: include/Label.h util/parse.h handling/Bexcept.h
formats/filePBN.obj: formats/filePBN.h formats/WriteInfo.h records/Segment.h
formats/filePBN.obj: records/Date.h records/Location.h records/Session.h
formats/filePBN.obj: records/Teams.h records/Team.h records/Scoring.h
formats/filePBN.obj: records/HeaderLIN.h records/LINData.h records/Board.h
formats/filePBN.obj: records/Deal.h include/bridge.h include/Format.h
formats/filePBN.obj: include/Label.h records/Tableau.h records/LeadTableau.h
formats/filePBN.obj: analysis/LeadTriple.h records/Instance.h
formats/filePBN.obj: records/Players.h records/Auction.h records/Contract.h
formats/filePBN.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
formats/filePBN.obj: analysis/LeadTriple.h analysis/dll.h records/PlayTrace.h
formats/filePBN.obj: analysis/PlayError.h records/GivenScore.h
formats/filePBN.obj: analysis/Valuation.h analysis/Term.h
formats/filePBN.obj: analysis/Composites.h records/Board.h files/Buffer.h
formats/filePBN.obj: files/LineData.h files/Chunk.h dispatch/Order.h
formats/filePBN.obj: include/Label.h handling/Bexcept.h
formats/fileRBN.obj: formats/fileRBN.h formats/WriteInfo.h records/Segment.h
formats/fileRBN.obj: records/Date.h records/Location.h records/Session.h
formats/fileRBN.obj: records/Teams.h records/Team.h records/Scoring.h
formats/fileRBN.obj: records/HeaderLIN.h records/LINData.h records/Board.h
formats/fileRBN.obj: records/Deal.h include/bridge.h include/Format.h
formats/fileRBN.obj: include/Label.h records/Tableau.h records/LeadTableau.h
formats/fileRBN.obj: analysis/LeadTriple.h records/Instance.h
formats/fileRBN.obj: records/Players.h records/Auction.h records/Contract.h
formats/fileRBN.obj: records/Play.h records/SDKnowledge.h analysis/ddsIF.h
formats/fileRBN.obj: analysis/LeadTriple.h analysis/dll.h records/PlayTrace.h
formats/fileRBN.obj: analysis/PlayError.h records/GivenScore.h
formats/fileRBN.obj: analysis/Valuation.h analysis/Term.h
formats/fileRBN.obj: analysis/Composites.h files/Buffer.h files/LineData.h
formats/fileRBN.obj: files/Chunk.h dispatch/Order.h include/Label.h
formats/fileRBN.obj: util/parse.h handling/Bexcept.h
formats/fileREC.obj: formats/fileREC.h formats/WriteInfo.h formats/Canvas.h
formats/fileREC.obj: records/Segment.h records/Date.h records/Location.h
formats/fileREC.obj: records/Session.h records/Teams.h records/Team.h
formats/fileREC.obj: records/Scoring.h records/HeaderLIN.h records/LINData.h
formats/fileREC.obj: records/Board.h records/Deal.h include/bridge.h
formats/fileREC.obj: include/Format.h include/Label.h records/Tableau.h
formats/fileREC.obj: records/LeadTableau.h analysis/LeadTriple.h
formats/fileREC.obj: records/Instance.h records/Players.h records/Auction.h
formats/fileREC.obj: records/Contract.h records/Play.h records/SDKnowledge.h
formats/fileREC.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
formats/fileREC.obj: records/PlayTrace.h analysis/PlayError.h
formats/fileREC.obj: records/GivenScore.h analysis/Valuation.h
formats/fileREC.obj: analysis/Term.h analysis/Composites.h records/Board.h
formats/fileREC.obj: files/Buffer.h files/LineData.h files/Chunk.h
formats/fileREC.obj: dispatch/Order.h include/Label.h util/parse.h
formats/fileREC.obj: handling/Bexcept.h
formats/fileTXT.obj: formats/fileTXT.h formats/WriteInfo.h formats/Canvas.h
formats/fileTXT.obj: records/Segment.h records/Date.h records/Location.h
formats/fileTXT.obj: records/Session.h records/Teams.h records/Team.h
formats/fileTXT.obj: records/Scoring.h records/HeaderLIN.h records/LINData.h
formats/fileTXT.obj: records/Board.h records/Deal.h include/bridge.h
formats/fileTXT.obj: include/Format.h include/Label.h records/Tableau.h
formats/fileTXT.obj: records/LeadTableau.h analysis/LeadTriple.h
formats/fileTXT.obj: records/Instance.h records/Players.h records/Auction.h
formats/fileTXT.obj: records/Contract.h records/Play.h records/SDKnowledge.h
formats/fileTXT.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
formats/fileTXT.obj: records/PlayTrace.h analysis/PlayError.h
formats/fileTXT.obj: records/GivenScore.h analysis/Valuation.h
formats/fileTXT.obj: analysis/Term.h analysis/Composites.h files/Buffer.h
formats/fileTXT.obj: files/LineData.h files/Chunk.h dispatch/Order.h
formats/fileTXT.obj: include/Label.h files/LineData.h util/parse.h
formats/fileTXT.obj: handling/Bexcept.h
handling/Bdiff.obj: handling/Bdiff.h
handling/Bexcept.obj: handling/Bexcept.h
records/Auction.obj: records/Auction.h include/bridge.h include/Format.h
records/Auction.obj: include/Label.h records/Contract.h util/parse.h
records/Auction.obj: handling/Bexcept.h handling/Bdiff.h
records/Board.obj: records/Board.h records/Deal.h include/bridge.h
records/Board.obj: include/Format.h include/Label.h records/Tableau.h
records/Board.obj: records/LeadTableau.h analysis/LeadTriple.h
records/Board.obj: records/Instance.h records/Players.h records/Auction.h
records/Board.obj: records/Contract.h records/Play.h records/SDKnowledge.h
records/Board.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
records/Board.obj: records/PlayTrace.h analysis/PlayError.h records/LINData.h
records/Board.obj: records/GivenScore.h analysis/Valuation.h analysis/Term.h
records/Board.obj: analysis/Composites.h util/parse.h handling/Bexcept.h
records/Board.obj: handling/Bdiff.h
records/Contract.obj: records/Contract.h include/bridge.h include/Format.h
records/Contract.obj: include/Label.h util/parse.h handling/Bexcept.h
records/Contract.obj: handling/Bdiff.h
records/Date.obj: records/Date.h util/parse.h include/bridge.h
records/Date.obj: include/Format.h include/Label.h handling/Bexcept.h
records/Date.obj: handling/Bdiff.h
records/Deal.obj: records/Deal.h include/bridge.h include/Format.h
records/Deal.obj: include/Label.h util/parse.h handling/Bexcept.h
records/Deal.obj: handling/Bdiff.h
records/GivenScore.obj: records/GivenScore.h util/parse.h include/bridge.h
records/GivenScore.obj: include/Format.h include/Label.h handling/Bexcept.h
records/GivenScore.obj: handling/Bdiff.h
records/Group.obj: records/Group.h records/Segment.h records/Date.h
records/Group.obj: records/Location.h records/Session.h records/Teams.h
records/Group.obj: records/Team.h records/Scoring.h records/HeaderLIN.h
records/Group.obj: records/LINData.h records/Board.h records/Deal.h
records/Group.obj: include/bridge.h include/Format.h include/Label.h
records/Group.obj: records/Tableau.h records/LeadTableau.h
records/Group.obj: analysis/LeadTriple.h records/Instance.h records/Players.h
records/Group.obj: records/Auction.h records/Contract.h records/Play.h
records/Group.obj: records/SDKnowledge.h analysis/ddsIF.h
records/Group.obj: analysis/LeadTriple.h analysis/dll.h records/PlayTrace.h
records/Group.obj: analysis/PlayError.h records/GivenScore.h
records/Group.obj: analysis/Valuation.h analysis/Term.h analysis/Composites.h
records/Group.obj: handling/Bdiff.h
records/HeaderLIN.obj: records/HeaderLIN.h records/LINData.h util/parse.h
records/HeaderLIN.obj: include/bridge.h include/Format.h include/Label.h
records/HeaderLIN.obj: handling/Bexcept.h
records/Instance.obj: records/Instance.h records/Players.h include/bridge.h
records/Instance.obj: include/Format.h include/Label.h records/Auction.h
records/Instance.obj: records/Contract.h records/Play.h records/SDKnowledge.h
records/Instance.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
records/Instance.obj: records/PlayTrace.h analysis/PlayError.h
records/Instance.obj: records/LINData.h util/parse.h handling/Bexcept.h
records/Instance.obj: handling/Bdiff.h
records/LeadTableau.obj: records/LeadTableau.h analysis/LeadTriple.h
records/LeadTableau.obj: include/bridge.h include/Format.h include/Label.h
records/LeadTableau.obj: records/Deal.h util/parse.h handling/Bexcept.h
records/LeadTableau.obj: handling/Bdiff.h
records/Location.obj: records/Location.h include/Format.h handling/Bexcept.h
records/Location.obj: handling/Bdiff.h
records/Play.obj: records/Play.h records/SDKnowledge.h include/bridge.h
records/Play.obj: include/Format.h include/Label.h analysis/ddsIF.h
records/Play.obj: analysis/LeadTriple.h analysis/dll.h records/Contract.h
records/Play.obj: records/SDPoints.h util/parse.h handling/Bexcept.h
records/Play.obj: handling/Bdiff.h
records/PlayTrace.obj: records/PlayTrace.h include/bridge.h include/Format.h
records/PlayTrace.obj: include/Label.h analysis/PlayError.h
records/PlayTrace.obj: handling/Bexcept.h handling/Bdiff.h
records/Players.obj: records/Players.h include/bridge.h include/Format.h
records/Players.obj: include/Label.h util/parse.h handling/Bexcept.h
records/Players.obj: handling/Bdiff.h
records/Scoring.obj: records/Scoring.h include/Format.h handling/Bexcept.h
records/Scoring.obj: handling/Bdiff.h
records/SDKnowledge.obj: records/SDKnowledge.h include/bridge.h
records/SDKnowledge.obj: include/Format.h include/Label.h handling/Bexcept.h
records/SDKnowledge.obj: handling/Bdiff.h
records/Segment.obj: records/Segment.h records/Date.h records/Location.h
records/Segment.obj: records/Session.h records/Teams.h records/Team.h
records/Segment.obj: records/Scoring.h records/HeaderLIN.h records/LINData.h
records/Segment.obj: records/Board.h records/Deal.h include/bridge.h
records/Segment.obj: include/Format.h include/Label.h records/Tableau.h
records/Segment.obj: records/LeadTableau.h analysis/LeadTriple.h
records/Segment.obj: records/Instance.h records/Players.h records/Auction.h
records/Segment.obj: records/Contract.h records/Play.h records/SDKnowledge.h
records/Segment.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
records/Segment.obj: records/PlayTrace.h analysis/PlayError.h
records/Segment.obj: records/GivenScore.h analysis/Valuation.h
records/Segment.obj: analysis/Term.h analysis/Composites.h util/parse.h
records/Segment.obj: handling/Bexcept.h handling/Bdiff.h
records/Session.obj: records/Session.h include/Format.h util/parse.h
records/Session.obj: include/bridge.h include/Format.h include/Label.h
records/Session.obj: handling/Bdiff.h handling/Bexcept.h
records/Sheet.obj: records/Sheet.h records/Contract.h include/bridge.h
records/Sheet.obj: include/Format.h include/Label.h records/Deal.h
records/Sheet.obj: records/Auction.h records/Play.h records/SDKnowledge.h
records/Sheet.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
records/Sheet.obj: records/SheetHand.h edits/Reflines.h edits/RefLine.h
records/Sheet.obj: edits/RefEdit.h edits/RefCount.h edits/refconst.h
records/Sheet.obj: include/Format.h edits/RefComment.h edits/RefEntry.h
records/Sheet.obj: edits/RefAction.h records/Segment.h records/Date.h
records/Sheet.obj: records/Location.h records/Session.h records/Teams.h
records/Sheet.obj: records/Team.h records/Scoring.h records/HeaderLIN.h
records/Sheet.obj: records/LINData.h records/Board.h records/Tableau.h
records/Sheet.obj: records/LeadTableau.h analysis/LeadTriple.h
records/Sheet.obj: records/Instance.h records/Players.h records/PlayTrace.h
records/Sheet.obj: analysis/PlayError.h records/GivenScore.h
records/Sheet.obj: analysis/Valuation.h analysis/Term.h analysis/Composites.h
records/Sheet.obj: files/Buffer.h files/LineData.h util/parse.h
records/Sheet.obj: handling/Bexcept.h handling/Bdiff.h
records/SheetHand.obj: records/SheetHand.h records/Contract.h
records/SheetHand.obj: include/bridge.h include/Format.h include/Label.h
records/SheetHand.obj: records/Deal.h records/Auction.h records/Play.h
records/SheetHand.obj: records/SDKnowledge.h analysis/ddsIF.h
records/SheetHand.obj: analysis/LeadTriple.h analysis/dll.h util/parse.h
records/SheetHand.obj: handling/Bexcept.h handling/Bdiff.h
records/Tableau.obj: records/Tableau.h include/bridge.h include/Format.h
records/Tableau.obj: include/Label.h records/Contract.h util/parse.h
records/Tableau.obj: handling/Bexcept.h handling/Bdiff.h
records/Team.obj: records/Team.h util/parse.h include/bridge.h
records/Team.obj: include/Format.h include/Label.h handling/Bexcept.h
records/Team.obj: handling/Bdiff.h
records/Teams.obj: records/Teams.h records/Team.h include/Format.h
records/Teams.obj: util/parse.h include/bridge.h include/Format.h
records/Teams.obj: include/Label.h handling/Bexcept.h handling/Bdiff.h
stats/AllStats.obj: stats/AllStats.h stats/ValStats.h stats/ValRow.h
stats/AllStats.obj: validate/ValProfile.h include/Format.h include/Label.h
stats/AllStats.obj: stats/TextStats.h stats/TextStat.h stats/TextDatum.h
stats/AllStats.obj: stats/CompStats.h stats/RefStats.h edits/RefEntry.h
stats/AllStats.obj: stats/DuplStats.h stats/DuplStat.h stats/ParamStats1D.h
stats/AllStats.obj: stats/Stats1D.h stats/StatsInfo.h stats/ParamStats2D.h
stats/AllStats.obj: stats/Stats2D.h stats/RuleStats.h stats/Rules.h
stats/AllStats.obj: stats/Rule.h stats/Timers.h stats/Timer.h
stats/AllStats.obj: analysis/Distributions.h control/Options.h
stats/AllStats.obj: analysis/DistMatcher.h analysis/DistElem.h
stats/CompStats.obj: stats/CompStats.h include/Format.h util/parse.h
stats/CompStats.obj: include/bridge.h include/Format.h include/Label.h
stats/DuplStat.obj: stats/DuplStat.h include/Format.h records/Group.h
stats/DuplStat.obj: records/Segment.h records/Date.h records/Location.h
stats/DuplStat.obj: records/Session.h records/Teams.h records/Team.h
stats/DuplStat.obj: records/Scoring.h records/HeaderLIN.h records/LINData.h
stats/DuplStat.obj: records/Board.h records/Deal.h include/bridge.h
stats/DuplStat.obj: include/Format.h include/Label.h records/Tableau.h
stats/DuplStat.obj: records/LeadTableau.h analysis/LeadTriple.h
stats/DuplStat.obj: records/Instance.h records/Players.h records/Auction.h
stats/DuplStat.obj: records/Contract.h records/Play.h records/SDKnowledge.h
stats/DuplStat.obj: analysis/ddsIF.h analysis/LeadTriple.h analysis/dll.h
stats/DuplStat.obj: records/PlayTrace.h analysis/PlayError.h
stats/DuplStat.obj: records/GivenScore.h analysis/Valuation.h analysis/Term.h
stats/DuplStat.obj: analysis/Composites.h edits/RefLines.h edits/RefLine.h
stats/DuplStat.obj: edits/RefEdit.h edits/RefCount.h edits/refconst.h
stats/DuplStat.obj: edits/RefComment.h edits/RefEntry.h edits/RefAction.h
stats/DuplStat.obj: util/parse.h handling/Bexcept.h
stats/DuplStats.obj: stats/DuplStats.h stats/DuplStat.h include/Format.h
stats/DuplStats.obj: handling/Bexcept.h
stats/RefStats.obj: stats/RefStats.h edits/RefEntry.h edits/Comment.h
stats/RefStats.obj: include/Format.h handling/Bexcept.h
stats/ParamStats1D.obj: stats/ParamStats1D.h stats/Stats1D.h
stats/ParamStats1D.obj: stats/StatsInfo.h
stats/ParamStats2D.obj: stats/ParamStats2D.h stats/Stats2D.h
stats/ParamStats2D.obj: stats/StatsInfo.h
stats/RuleStats.obj: analysis/passes/PassTables.h analysis/passes/PassTable.h
stats/RuleStats.obj: analysis/passes/PassRow.h analysis/passes/PassTerm.h
stats/RuleStats.obj: analysis/passes/SigmoidData.h analysis/Distributions.h
stats/RuleStats.obj: stats/RowData.h include/bridge.h include/Format.h
stats/RuleStats.obj: include/Label.h stats/RuleStats.h stats/Rules.h
stats/RuleStats.obj: stats/Rule.h
stats/Rules.obj: stats/Rules.h stats/Rule.h stats/RowData.h
stats/Rule.obj: stats/Rule.h stats/RowData.h
stats/Stats1D.obj: stats/Stats1D.h stats/StatsInfo.h
stats/Stats2D.obj: stats/Stats2D.h stats/StatsInfo.h
stats/TextDatum.obj: stats/TextDatum.h
stats/TextStat.obj: stats/TextStat.h stats/TextDatum.h
stats/TextStats.obj: stats/TextStats.h stats/TextStat.h stats/TextDatum.h
stats/TextStats.obj: include/Format.h include/Label.h
stats/Timer.obj: stats/Timer.h
stats/Timers.obj: stats/Timers.h stats/Timer.h include/Format.h
stats/ValRow.obj: stats/ValRow.h validate/ValProfile.h include/Format.h
stats/ValRow.obj: include/Label.h
stats/ValStats.obj: stats/ValStats.h stats/ValRow.h validate/ValProfile.h
stats/ValStats.obj: include/Format.h include/Label.h
util/parse.obj: util/parse.h include/bridge.h include/Format.h
util/parse.obj: include/Label.h
validate/ValProfile.obj: validate/ValProfile.h files/LineData.h
validate/validate.obj: validate/validateLIN.h validate/validatePBN.h
validate/validate.obj: validate/validateRBN.h validate/validateTXT.h
validate/validate.obj: validate/validateEML.h validate/validateREC.h
validate/validate.obj: validate/valint.h files/Buffer.h files/LineData.h
validate/validate.obj: validate/ValProfile.h control/Options.h
validate/validate.obj: analysis/DistMatcher.h analysis/DistElem.h
validate/validate.obj: files/Chunk.h dispatch/Order.h include/Label.h
validate/validate.obj: files/LineData.h formats/filePBN.h edits/RefLines.h
validate/validate.obj: edits/RefLine.h edits/RefEdit.h edits/RefCount.h
validate/validate.obj: edits/refconst.h include/Format.h edits/RefComment.h
validate/validate.obj: edits/RefEntry.h edits/RefAction.h stats/ValStats.h
validate/validate.obj: stats/ValRow.h validate/ValProfile.h util/parse.h
validate/validate.obj: include/bridge.h include/Format.h include/Label.h
validate/validateEML.obj: validate/validate.h validate/valint.h
validate/validateEML.obj: files/Buffer.h files/LineData.h
validate/validateEML.obj: validate/ValProfile.h validate/validateEML.h
validate/validateLIN.obj: validate/valint.h files/Buffer.h files/LineData.h
validate/validateLIN.obj: validate/ValProfile.h validate/validate.h
validate/validateLIN.obj: validate/validateLIN.h records/Deal.h
validate/validateLIN.obj: include/bridge.h include/Format.h include/Label.h
validate/validateLIN.obj: util/parse.h handling/Bexcept.h handling/Bdiff.h
validate/validatePBN.obj: validate/validate.h validate/valint.h
validate/validatePBN.obj: files/Buffer.h files/LineData.h
validate/validatePBN.obj: validate/validatePBN.h validate/ValProfile.h
validate/validatePBN.obj: files/Chunk.h dispatch/Order.h include/Label.h
validate/validatePBN.obj: util/parse.h include/bridge.h include/Format.h
validate/validatePBN.obj: include/Label.h
validate/validateRBN.obj: validate/validate.h validate/valint.h
validate/validateRBN.obj: files/Buffer.h files/LineData.h
validate/validateRBN.obj: validate/validateRBN.h validate/ValProfile.h
validate/validateRBN.obj: util/parse.h include/bridge.h include/Format.h
validate/validateRBN.obj: include/Label.h
validate/validateREC.obj: validate/validate.h validate/valint.h
validate/validateREC.obj: files/Buffer.h files/LineData.h
validate/validateREC.obj: validate/validateREC.h validate/ValProfile.h
validate/validateREC.obj: util/parse.h include/bridge.h include/Format.h
validate/validateREC.obj: include/Label.h
validate/validateTXT.obj: validate/validate.h validate/valint.h
validate/validateTXT.obj: files/Buffer.h files/LineData.h
validate/validateTXT.obj: validate/validateTXT.h validate/ValProfile.h
validate/validateTXT.obj: util/parse.h include/bridge.h include/Format.h
validate/validateTXT.obj: include/Label.h
reader.obj: control/args.h control/Options.h analysis/DistMatcher.h
reader.obj: analysis/DistElem.h files/Files.h files/FileTask.h
reader.obj: include/Format.h analysis/DDInfo.h dispatch/dispatch.h
reader.obj: dispatch/funcPasses.h stats/AllStats.h stats/Timer.h
