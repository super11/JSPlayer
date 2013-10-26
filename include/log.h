
void log( const char *fileName, int line, const char *msg, ... )
{
      va_list args;
      va_start( args, msg );

      fprintf( stdout, "%s, %d:", fileName, line );
      vfprintf( stdout, msg, args );
      fprintf( stdout, "n" );

      va_end( args );
}

#define LOG(msg, ...) log(__FILE__, __LINE__, msg, ##__VA_ARGS__)
