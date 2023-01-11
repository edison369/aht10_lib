project(CFE_AHT10 C)

# Create the app module
add_cfe_app(aht10 fsw/src/aht10.c)

# The API to this library (which may be invoked/referenced from other apps)
# is stored in fsw/public_inc.
target_include_directories(aht10 PUBLIC fsw/public_inc)
