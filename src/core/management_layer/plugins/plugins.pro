TEMPLATE = subdirs

SUBDIRS = \
    comic_book_information \
    comic_book_parameters \
    comic_book_text \
    comic_book_text_structure \
    project_information \
    screenplay_information \
    screenplay_parameters \
    screenplay_treatment \
    screenplay_text \
    screenplay_text_structure \
    simple_text \
    simple_text_structure \
    title_page

exists (characters_relations/characters_relations.pro) {
    SUBDIRS += characters_relations
}

exists (character_information/character_information.pro) {
    SUBDIRS += character_information
}

exists (locations_map/locations_map.pro) {
    SUBDIRS += locations_map
}

exists (location_information/location_information.pro) {
    SUBDIRS += location_information
}

exists (screenplay_statistics/screenplay_statistics.pro) {
    SUBDIRS += screenplay_statistics
}

exists (comic_book_statistics/comic_book_statistics.pro) {
    SUBDIRS += comic_book_statistics
}
