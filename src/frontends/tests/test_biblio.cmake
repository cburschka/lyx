# check of output of biblio-command with the saved data in regfiles/biblio
#
# Input variable
# biblio       = full path of the biblio executable
# saved_data   = full path of the file with expected output of biblio
# 
if(NOT EXISTS "${biblio}")
  message(STATUS "searching for biblio as ${biblio}")
  message(FATAL_ERROR "Compile biblio first")
else()
  execute_process(COMMAND "${biblio}" OUTPUT_VARIABLE biblio_out RESULT_VARIABLE biblio_res)

  file(READ "${saved_data}" biblio_check)
  if(NOT biblio_out STREQUAL biblio_check)
    message(FATAL_ERROR "biblio_out = ${biblio_out}\nbiblio_in = ${biblio_in}")
  endif()
endif()
