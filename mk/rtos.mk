RTOS_OBJDIR := ./bin
RTOS_SRC	:= $(wildcard *.c)
RTOS_OBJ	:= $(patsubst %.c,$(RTOS_OBJDIR)/%.o,$(RTOS_SRC))

rtos: $(RTOS_OBJ)
	#echo $(RTOS_OBJ)
	echo "Hallo"

$(RTOS_OBJ):  | $(RTOS_OBJDIR)
	
$(RTOS_OBJDIR):
	mkdir $(RTOS_OBJDIR)

.PHONY: rtos_clean

rtos_clean:
	-rm $(RTOS_OBJ)
	