# Copyright (C) 2025  <name of copyright holder>
# Author: Linas <linas@justforfun.cn
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set(LOCAL_FILES
    check_result.c
    compile.c
    main.c
    judge.c
    result.c
    server.c
    server.h
    sys_info.c
    sys_info.h
    rules/c_cpp.c
    limit.c
)

add_executable(myoj-judge-server ${LOCAL_FILES})
target_link_libraries(myoj-judge-server PRIVATE libseccomp.so)

install(TARGETS myoj-judge-server)
