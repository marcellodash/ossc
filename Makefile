.PHONY: clean All

All:
	@echo "----------Building project:[ tools - Release ]----------"
	@"$(MAKE)" -f  "tools.mk"
clean:
	@echo "----------Cleaning project:[ tools - Release ]----------"
	@"$(MAKE)" -f  "tools.mk" clean
