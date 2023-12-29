# Copyright (©) 2023  Frosty515
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

.PHONY: clean-programs build-programs distclean-programs install-programs

clean-programs:
	@$(MAKE) -C Utilities clean-utilities

build-programs:
	@echo ------------------
	@echo Building Utilities
	@echo ------------------
	@$(MAKE) -C Utilities build-utilities

distclean-programs:
	@$(MAKE) -C Utilities distclean-utilities

install-programs:
	@echo --------------------
	@echo Installing Utilities
	@echo --------------------
	@$(MAKE) -C Utilities install-utilities
