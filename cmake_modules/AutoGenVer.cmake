
function(modify_project_version)
    set( PROJECT_STORE_PATH ${PROJECT_SOURCE_DIR})
    string(TIMESTAMP PROJECT_BUILD_YEAR "%Y")
    string(TIMESTAMP PROJECT_BUILD_DATE "%Y%m%d")
    string(TIMESTAMP PROJECT_BUILD_TIME "%H%M%S")

    # get svn/git commit reversion
    if(EXISTS "${PROJECT_STORE_PATH}/.git/")
        find_package(Git)
        if(GIT_FOUND)
            execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE res_code OUTPUT_VARIABLE GIT_COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
           
            # 获取 Git 提交哈希（短格式）
            execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
                OUTPUT_VARIABLE GIT_HASH  OUTPUT_STRIP_TRAILING_WHITESPACE    )

            # message("aaaaa GIT_HASH  --- ${GIT_HASH}")
            # 获取当前分支名称
            execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                            OUTPUT_VARIABLE GIT_BRANCH   OUTPUT_STRIP_TRAILING_WHITESPACE )
           
            # 获取最近的 Git 标签
            execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_TAG      OUTPUT_STRIP_TRAILING_WHITESPACE  ERROR_QUIET )
            # message(" GIT_TAG  --- ${GIT_TAG}")

            # 获取提交信息标题
            execute_process(
                COMMAND git log -1 --pretty=format:%s
                OUTPUT_VARIABLE COMMIT_INFO
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
           
            if(${res_code} EQUAL 0)
                # message("------  Get git revision success  ------")
                # message("------  GIT_COMMIT_ID   --- ${GIT_COMMIT_ID}")
                # message("------  GIT_BRANCH  --- ${GIT_BRANCH}")
                # message("------  COMMIT_INFO  --- ${COMMIT_INFO}")
                # -g: tag of git
                string(FIND  ${GIT_COMMIT_ID} "-g" pos)
                if(${pos} GREATER 0)
                    string(SUBSTRING ${GIT_COMMIT_ID} ${pos} -1 COMMIT_ID)
                    string(SUBSTRING ${COMMIT_ID} 2 -1 PROJECT_REVISION)
                    # message("-- Git commit id: ${PROJECT_REVISION}")
                endif()
            else(${res_code} EQUAL 0)
                message( WARNING "-- Git failed (not a repo, or no tags). Build will not contain git revision info." )
            endif(${res_code} EQUAL 0)
        else(GIT_FOUND)
            message("-- Git not found!)")
        endif(GIT_FOUND)
    else(EXISTS "${PROJECT_STORE_PATH}/.git/")
        if(EXISTS "${PROJECT_STORE_PATH}/.svn/")
            FIND_PACKAGE(Subversion)
            if(SUBVERSION_FOUND)
                Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} Project)
                SET(PROJECT_REVISION ${Project_WC_REVISION})
                message("-- Svn revision:${PROJECT_REVISION}")
            else(SUBVERSION_FOUND)
                message("-- Can't find packet Subversion")
            endif(SUBVERSION_FOUND)
        else()
            message(ERROR "-- Svn directory not exists")
        endif(EXISTS "${PROJECT_STORE_PATH}/.svn/")
    endif(EXISTS "${PROJECT_STORE_PATH}/.git/")

    # generate the version file
    set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ver.h)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ver.h.in"   "${VERSION_FILE}"  @ONLY)

endfunction()
