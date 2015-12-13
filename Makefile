.PHONY: clean All

All:
	@echo "----------Building project:[ librudp - Release ]----------"
	@"$(MAKE)" -f  "librudp.mk" && "$(MAKE)" -f  "librudp.mk" PostBuild
	@echo "----------Building project:[ server - Release ]----------"
	@cd "server" && "$(MAKE)" -f  "server.mk" && "$(MAKE)" -f  "server.mk" PostBuild
	@echo "----------Building project:[ client - Release ]----------"
	@cd "client" && "$(MAKE)" -f  "client.mk" && "$(MAKE)" -f  "client.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ librudp - Release ]----------"
	@"$(MAKE)" -f  "librudp.mk"  clean
	@echo "----------Cleaning project:[ server - Release ]----------"
	@cd "server" && "$(MAKE)" -f  "server.mk"  clean
	@echo "----------Cleaning project:[ client - Release ]----------"
	@cd "client" && "$(MAKE)" -f  "client.mk" clean
