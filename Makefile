lint: cpplint shlint javalint pylint

format:
	@bash tools/format.sh

cpplint:
	@git ls-files | grep --regexp "\(\.h\|\.cc\)$$" | xargs -I {} cpplint {}

shlint:
	@git ls-files | grep --regexp "\.sh$$" | xargs -I {} shellcheck {}

javalint:
	@cd java && mvn checkstyle:checkstyle

pylint:
	@git ls-files | grep --regexp "\.py$$" | xargs -I {} pylint {}
