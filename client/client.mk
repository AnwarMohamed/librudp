##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=client
ConfigurationName      :=Debug
WorkspacePath          := "/media/Coding/Github/librudp"
ProjectPath            := "/media/Coding/Github/librudp/client"
IntermediateDirectory  :=./../build/debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Anwar Mohamed
Date                   :=12/13/15
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
OutputFile             :=./../rudp-client
Preprocessors          :=$(PreprocessorSwitch)DEBUG_MODE $(PreprocessorSwitch)_POSIX_TIMERS 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="client.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)./../ 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)rudp $(LibrarySwitch)pthread $(LibrarySwitch)event $(LibrarySwitch)rt 
ArLibs                 :=  "rudp" "pthread" "event" "rt" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../ 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -Wall -std=c11 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall -std=c11 -D_POSIX_C_SOURCE=200809L $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d "../.build-debug/librudp" "../.build-debug/server" $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

"../.build-debug/librudp":
	@$(MakeDirCommand) "../.build-debug"
	@echo stam > "../.build-debug/librudp"


"../.build-debug/server":
	@$(MakeDirCommand) "../.build-debug"
	@echo stam > "../.build-debug/server"




MakeIntermediateDirs:
	@test -d ./../build/debug || $(MakeDirCommand) ./../build/debug


$(IntermediateDirectory)/.d:
	@test -d ./../build/debug || $(MakeDirCommand) ./../build/debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.c$(ObjectSuffix): main.c $(IntermediateDirectory)/main.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/media/Coding/Github/librudp/client/main.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.c$(DependSuffix): main.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.c$(ObjectSuffix) -MF$(IntermediateDirectory)/main.c$(DependSuffix) -MM "main.c"

$(IntermediateDirectory)/main.c$(PreprocessSuffix): main.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.c$(PreprocessSuffix) "main.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./../build/debug/


