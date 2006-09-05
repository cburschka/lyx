/*File generated during static filter build
  Automatically generated file
*/

  extern "C" IndividualFilter * new_aspell_url_filter();

  static FilterEntry standard_filters[] = {
    {"url",0,new_aspell_url_filter,0}
  };

  const unsigned int standard_filters_size = sizeof(standard_filters)/sizeof(FilterEntry);

  static KeyInfo url_options[] = {
	""
  };

  const KeyInfo * url_options_begin = url_options;

  const KeyInfo * url_options_end = url_options+sizeof(url_options)/sizeof(KeyInfo);


  static ConfigModule filter_modules[] = {
    {
      "url",0,
      "filter to skip URL like constructs",
      url_options_begin,url_options_end
    }
  };

  const ConfigModule * filter_modules_begin = filter_modules;

  const ConfigModule * filter_modules_end = filter_modules+sizeof(filter_modules)/sizeof(ConfigModule);

  const size_t filter_modules_size = sizeof(filter_modules);


