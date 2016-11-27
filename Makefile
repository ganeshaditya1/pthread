OBJDIR = build
targets = timer queue scheduler my_pthread_t context threadStructure multiLevelQueue my_malloc
properTargets = $(addsuffix .o, $(addprefix $(OBJDIR)/, $(targets)))

.PHONY: dirs 
dirs: 
	make clean
	mkdir $(OBJDIR)
	make all
all: $(targets)
	gcc $(properTargets) demo.c -o $(OBJDIR)/demo -lm
my_malloc:
	gcc -c $@.c -o $(OBJDIR)/$@.o
timer:
	gcc -c $@.c -o $(OBJDIR)/$@.o
threadStructure:
	gcc -c $@.c -o $(OBJDIR)/$@.o
queue:
	gcc -c $@.c -o $(OBJDIR)/$@.o
scheduler: 
	gcc -c $@.c -o $(OBJDIR)/$@.o
my_pthread_t: 
	gcc -c $@.c -o $(OBJDIR)/$@.o
context:
	gcc -c $@.c -o $(OBJDIR)/$@.o
multiLevelQueue:
	gcc -c $@.c -o $(OBJDIR)/$@.o
clean:
	rm -rf $(OBJDIR)