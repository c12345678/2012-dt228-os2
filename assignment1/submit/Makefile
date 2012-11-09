submit:
	@./submit.rb http://dit.briangillespie.net $(ID) $(PIN) 2>/dev/null || \
		echo "You must be conneted to the Internent provide a valid ID and PIN. Use: $$ make ID=yourStudentID PIN=yourPIN"

.PHONY: clean

clean:
	rm -rf *~ *.tar.gz assignment1 cookie.txt
