#--------------------------------------------------------------
# Additional include paths

INCLUDE_DIRECTORIES( ${CH_IRRLICHTINC} )


#--------------------------------------------------------------

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	SET (CH_LINKERFLAG_EXE  "${CH_LINKERFLAG_EXE} -framework IOKit -framework Cocoa -framework OpenGL")
ENDIF()

# Add executables

ADD_EXECUTABLE(demo_collision   	demo_collision.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_collision.cpp)
SET_TARGET_PROPERTIES(demo_collision PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	
	)
TARGET_LINK_LIBRARIES(demo_collision
	${CH_IRRLICHTLIB}
 	ChronoEngine)
ADD_DEPENDENCIES (demo_collision ChronoEngine)


ADD_EXECUTABLE(demo_benchmark   	demo_benchmark.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_benchmark.cpp)
SET_TARGET_PROPERTIES(demo_benchmark PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_benchmark
	${CH_IRRLICHTLIB}
 	ChronoEngine)
ADD_DEPENDENCIES (demo_benchmark ChronoEngine)

 	
ADD_EXECUTABLE(demo_bricks   	    demo_bricks.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_bricks.cpp)
SET_TARGET_PROPERTIES(demo_bricks PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_bricks
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_bricks ChronoEngine)

 	
ADD_EXECUTABLE(demo_cohesion   	    demo_cohesion.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_cohesion.cpp)
SET_TARGET_PROPERTIES(demo_cohesion PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_cohesion
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_cohesion ChronoEngine)


ADD_EXECUTABLE(demo_conveyor   	    demo_conveyor.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_conveyor.cpp)
SET_TARGET_PROPERTIES(demo_conveyor PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_conveyor
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_conveyor ChronoEngine)


ADD_EXECUTABLE(demo_crank   	    demo_crank.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_crank.cpp)
SET_TARGET_PROPERTIES(demo_crank PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_crank
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_crank ChronoEngine)

 	
ADD_EXECUTABLE(demo_decomposition   demo_decomposition.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_decomposition.cpp)
SET_TARGET_PROPERTIES(demo_decomposition PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_decomposition
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_decomposition ChronoEngine)

 	
ADD_EXECUTABLE(demo_forklift    	demo_forklift.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_forklift.cpp)
SET_TARGET_PROPERTIES(demo_forklift PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_forklift
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_forklift ChronoEngine)

 	
ADD_EXECUTABLE(demo_fourbar    		demo_fourbar.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_fourbar.cpp)
SET_TARGET_PROPERTIES(demo_fourbar PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_fourbar
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_fourbar ChronoEngine)

 	
ADD_EXECUTABLE(demo_friction   		demo_friction.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_friction.cpp)
SET_TARGET_PROPERTIES(demo_friction PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_friction
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_friction ChronoEngine)

 	
ADD_EXECUTABLE(demo_gears   		demo_gears.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_gears.cpp)
SET_TARGET_PROPERTIES(demo_gears PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_gears
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_gears ChronoEngine)

 	
ADD_EXECUTABLE(demo_mecanum   		demo_mecanum.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_mecanum.cpp)
SET_TARGET_PROPERTIES(demo_mecanum PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_mecanum
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_mecanum ChronoEngine)

 	
ADD_EXECUTABLE(demo_pendulum   		demo_pendulum.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_pendulum.cpp)
SET_TARGET_PROPERTIES(demo_pendulum PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_pendulum
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_pendulum ChronoEngine)

 	
ADD_EXECUTABLE(demo_sph   		    demo_sph.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_sph.cpp)
SET_TARGET_PROPERTIES(demo_sph PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_sph
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_sph ChronoEngine)

 	
ADD_EXECUTABLE(demo_suspension   	demo_suspension.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_suspension.cpp)
SET_TARGET_PROPERTIES(demo_suspension PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_suspension
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_suspension ChronoEngine)


ADD_EXECUTABLE(demo_tire   			demo_tire.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_tire.cpp)
SET_TARGET_PROPERTIES(demo_tire PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_tire
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_tire ChronoEngine)

 	
ADD_EXECUTABLE(demo_tracks   		demo_tracks.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_tracks.cpp)
SET_TARGET_PROPERTIES(demo_tracks PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_tracks
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_tracks ChronoEngine)


ADD_EXECUTABLE(demo_convergence   		demo_convergence.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_convergence.cpp)
SET_TARGET_PROPERTIES(demo_convergence PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_convergence
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_convergence ChronoEngine)


ADD_EXECUTABLE(demo_irr_assets   	demo_irr_assets.cpp)
SOURCE_GROUP(demos\\irrlicht FILES  demo_irr_assets.cpp)
SET_TARGET_PROPERTIES(demo_irr_assets PROPERTIES 
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}"
	)
TARGET_LINK_LIBRARIES(demo_irr_assets
	${CH_IRRLICHTLIB} 
 	ChronoEngine)
ADD_DEPENDENCIES (demo_irr_assets ChronoEngine)

# Justin 170613, adding soil bin to the demos folder
ADD_EXECUTABLE(demo_soilbin		demo_soilbin.cpp)
SOURCE_GROUP(demos\\irrlicht FILES demo_soilbin.cpp)
SET_TARGET_PROPERTIES(demo_soilbin PROPERTIES
	FOLDER demos
	COMPILE_FLAGS "${CH_BUILDFLAGS}"
	LINK_FLAGS "${CH_LINKERFLAG_EXE}")
TARGET_LINK_LIBRARIES(demo_soilbin
	${CH_IRRLICHTLIB}
	ChronoEngine)
ADD_DEPENDENCIES(demo_soilbin ChronoEngine)

install(TARGETS demo_collision DESTINATION bin)
install(TARGETS demo_benchmark DESTINATION bin)
install(TARGETS demo_bricks DESTINATION bin)
install(TARGETS demo_cohesion DESTINATION bin)
install(TARGETS demo_conveyor DESTINATION bin)
install(TARGETS demo_crank DESTINATION bin)
install(TARGETS demo_decomposition DESTINATION bin)
install(TARGETS demo_forklift DESTINATION bin)
install(TARGETS demo_fourbar DESTINATION bin)
install(TARGETS demo_friction DESTINATION bin)
install(TARGETS demo_gears DESTINATION bin)
install(TARGETS demo_mecanum DESTINATION bin)
install(TARGETS demo_pendulum DESTINATION bin)
install(TARGETS demo_sph DESTINATION bin)
install(TARGETS demo_suspension DESTINATION bin)
install(TARGETS demo_tire DESTINATION bin)
install(TARGETS demo_tracks DESTINATION bin)
install(TARGETS demo_convergence DESTINATION bin)
install(TARGETS demo_irr_assets DESTINATION bin)
install(TARGETS demo_soilbin	DESTINATION bin)





