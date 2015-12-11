##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=librudp
ConfigurationName      :=Debug
WorkspacePath          := "/media/Coding/Github/librudp"
ProjectPath            := "/media/Coding/Github/librudp"
IntermediateDirectory  :=./build/debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Anwar Mohamed
Date                   :=12/11/15
CodeLitePath           :="/home/anwarelmakrahy/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=./$(ProjectName).so
Preprocessors          :=$(PreprocessorSwitch)DEBUG_MODE 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="librudp.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)pthread $(LibrarySwitch)event $(LibrarySwitch)rt $(LibrarySwitch)ssl 
ArLibs                 :=  "pthread" "event" "rt" "ssl" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -std=c11 -fPIC $(Preprocessors)
CFLAGS   :=  -g -std=c11 -fPIC -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/socket.c$(ObjectSuffix) $(IntermediateDirectory)/linkedlist.c$(ObjectSuffix) $(IntermediateDirectory)/queue.c$(ObjectSuffix) $(IntermediateDirectory)/channel.c$(ObjectSuffix) $(IntermediateDirectory)/packet.c$(ObjectSuffix) $(IntermediateDirectory)/utils.c$(ObjectSuffix) $(IntermediateDirectory)/window.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@$(MakeDirCommand) "/media/Coding/Github/librudp/.build-debug"
	@echo rebuilt > "/media/Coding/Github/librudp/.build-debug/librudp"

MakeIntermediateDirs:
	@test -d ./build/debug || $(MakeDirCommand) ./build/debug


$(IntermediateDirectory)/.d:
	@test -d ./build/debug || $(MakeDirCommand) ./build/debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/socket.c$(ObjectSuffix): socket.c $(IntermediateDirectory)/socket.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/socket.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/socket.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/socket.c$(DependSuffix): socket.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/socket.c$(ObjectSuffix) -MF$(IntermediateDirectory)/socket.c$(DependSuffix) -MM "socket.c"

$(IntermediateDirectory)/socket.c$(PreprocessSuffix): socket.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/socket.c$(PreprocessSuffix) "socket.c"

$(IntermediateDirectory)/linkedlist.c$(ObjectSuffix): linkedlist.c $(IntermediateDirectory)/linkedlist.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/linkedlist.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/linkedlist.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/linkedlist.c$(DependSuffix): linkedlist.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/linkedlist.c$(ObjectSuffix) -MF$(IntermediateDirectory)/linkedlist.c$(DependSuffix) -MM "linkedlist.c"

$(IntermediateDirectory)/linkedlist.c$(PreprocessSuffix): linkedlist.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/linkedlist.c$(PreprocessSuffix) "linkedlist.c"

$(IntermediateDirectory)/queue.c$(ObjectSuffix): queue.c $(IntermediateDirectory)/queue.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/queue.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/queue.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/queue.c$(DependSuffix): queue.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/queue.c$(ObjectSuffix) -MF$(IntermediateDirectory)/queue.c$(DependSuffix) -MM "queue.c"

$(IntermediateDirectory)/queue.c$(PreprocessSuffix): queue.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/queue.c$(PreprocessSuffix) "queue.c"

$(IntermediateDirectory)/channel.c$(ObjectSuffix): channel.c $(IntermediateDirectory)/channel.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/channel.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/channel.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/channel.c$(DependSuffix): channel.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/channel.c$(ObjectSuffix) -MF$(IntermediateDirectory)/channel.c$(DependSuffix) -MM "channel.c"

$(IntermediateDirectory)/channel.c$(PreprocessSuffix): channel.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/channel.c$(PreprocessSuffix) "channel.c"

$(IntermediateDirectory)/packet.c$(ObjectSuffix): packet.c $(IntermediateDirectory)/packet.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/packet.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/packet.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/packet.c$(DependSuffix): packet.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/packet.c$(ObjectSuffix) -MF$(IntermediateDirectory)/packet.c$(DependSuffix) -MM "packet.c"

$(IntermediateDirectory)/packet.c$(PreprocessSuffix): packet.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/packet.c$(PreprocessSuffix) "packet.c"

$(IntermediateDirectory)/utils.c$(ObjectSuffix): utils.c $(IntermediateDirectory)/utils.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/utils.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/utils.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/utils.c$(DependSuffix): utils.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/utils.c$(ObjectSuffix) -MF$(IntermediateDirectory)/utils.c$(DependSuffix) -MM "utils.c"

$(IntermediateDirectory)/utils.c$(PreprocessSuffix): utils.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/utils.c$(PreprocessSuffix) "utils.c"

$(IntermediateDirectory)/window.c$(ObjectSuffix): window.c $(IntermediateDirectory)/window.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/window.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/window.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/window.c$(DependSuffix): window.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/window.c$(ObjectSuffix) -MF$(IntermediateDirectory)/window.c$(DependSuffix) -MM "window.c"

$(IntermediateDirectory)/window.c$(PreprocessSuffix): window.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/window.c$(PreprocessSuffix) "window.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./build/debug/


