TEMPLATE = subdirs

clip2tri.subdir = 3rdparty/clip2tri
poly2tri.subdir = 3rdparty/poly2tri
clipper.subdir = 3rdparty/clipper

SUBDIRS += clip2tri clipper poly2tri
clip2tri.depends = clipper poly2tri

SUBDIRS += positioning
positioning.depends = clip2tri

qtHaveModule(quick) {
    SUBDIRS += location
    location.depends += positioning clip2tri

    plugins.depends += location

    SUBDIRS += imports
    imports.depends += positioning location
}

plugins.depends += positioning
SUBDIRS += plugins

!android:contains(QT_CONFIG, private_tests) {
    SUBDIRS += positioning_doc_snippets
    positioning_doc_snippets.subdir = positioning/doc/snippets

    #plugin dependency required during static builds
    positioning_doc_snippets.depends = positioning plugins

    qtHaveModule(quick) {
        SUBDIRS += location_doc_snippets
        location_doc_snippets.subdir = location/doc/snippets

        #plugin dependency required during static builds
        location_doc_snippets.depends = location plugins
    }
}
