#ifndef OSMIUM_OSM_OBJECT_HPP
#define OSMIUM_OSM_OBJECT_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2021 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <osmium/memory/collection.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/memory/item_iterator.hpp>
#include <osmium/osm/entity.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/misc.hpp>

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace osmium {

    namespace builder {
        template <typename TDerived, typename T>
        class OSMObjectBuilder;
    } // namespace builder

    /**
     * OSMObject (Node, Way, Relation, or Area).
     */
    class OSMObject : public osmium::OSMEntity {

        template <typename TDerived, typename T>
        friend class osmium::builder::OSMObjectBuilder;

        object_id_type      m_id = 0;
        bool                m_deleted : 1;
        object_version_type m_version : 31;
        osmium::Timestamp   m_timestamp{};
        user_id_type        m_uid = 0;
        changeset_id_type   m_changeset = 0;

        size_t sizeof_object() const noexcept {
            return sizeof(OSMObject) + (type() == item_type::node ? sizeof(osmium::Location) : 0) + sizeof(string_size_type);
        }

        unsigned char* user_position() noexcept {
            return data() + sizeof_object() - sizeof(string_size_type);
        }

        const unsigned char* user_position() const noexcept {
            return data() + sizeof_object() - sizeof(string_size_type);
        }

        string_size_type user_size() const noexcept {
            return *reinterpret_cast<const string_size_type*>(user_position());
        }

        unsigned char* subitems_position() {
            return data() + osmium::memory::padded_length(sizeof_object() + user_size());
        }

        const unsigned char* subitems_position() const {
            return data() + osmium::memory::padded_length(sizeof_object() + user_size());
        }

    protected:

        OSMObject(osmium::memory::item_size_type size, osmium::item_type type) :
            OSMEntity(size, type),
            m_deleted(false),
            m_version(0) {
        }

        void set_user_size(string_size_type size) {
            *reinterpret_cast<string_size_type*>(user_position()) = size;
        }

    public:

        constexpr static bool is_compatible_to(osmium::item_type t) noexcept {
            return t == osmium::item_type::node ||
                   t == osmium::item_type::way ||
                   t == osmium::item_type::relation ||
                   t == osmium::item_type::area;
        }

        /// Get ID of this object.
        object_id_type id() const noexcept {
            return m_id;
        }

        /// Get absolute value of the ID of this object.
        unsigned_object_id_type positive_id() const noexcept {
            return static_cast<unsigned_object_id_type>(std::abs(m_id));
        }

        /**
         * Set ID of this object.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_id(object_id_type id) noexcept {
            m_id = id;
            return *this;
        }

        /**
         * Set ID of this object.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_id(const char* id) {
            return set_id(osmium::string_to_object_id(id));
        }

        /// Is this object marked as deleted?
        bool deleted() const noexcept {
            return m_deleted;
        }

        /// Is this object marked visible (ie not deleted)?
        bool visible() const noexcept {
            return !deleted();
        }

        /**
         * Mark this object as deleted (or not).
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_deleted(bool deleted) noexcept {
            m_deleted = deleted;
            return *this;
        }

        /**
         * Mark this object as visible (ie not deleted) (or not).
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_visible(bool visible) noexcept {
            m_deleted = !visible;
            return *this;
        }

        /**
         * Mark this object as visible (ie not deleted) or deleted.
         *
         * @param visible Either "true" or "false"
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_visible(const char* visible) {
            if (!std::strcmp("true", visible)) {
                set_visible(true);
            } else if (!std::strcmp("false", visible)) {
                set_visible(false);
            } else {
                throw std::invalid_argument{"Unknown value for visible attribute (allowed is 'true' or 'false')"};
            }
            return *this;
        }

        /// Get version of this object.
        object_version_type version() const noexcept {
            return m_version;
        }

        /**
         * Set object version.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_version(object_version_type version) noexcept {
            m_version = version;
            return *this;
        }

        /**
         * Set object version.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_version(const char* version) {
            return set_version(string_to_object_version(version));
        }

        /// Get changeset id of this object.
        changeset_id_type changeset() const noexcept {
            return m_changeset;
        }

        /**
         * Set changeset id of this object.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_changeset(changeset_id_type changeset) noexcept {
            m_changeset = changeset;
            return *this;
        }

        /**
         * Set changeset id of this object.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_changeset(const char* changeset) {
            return set_changeset(string_to_changeset_id(changeset));
        }

        /// Get user id of this object.
        user_id_type uid() const noexcept {
            return m_uid;
        }

        /**
         * Set user id of this object.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_uid(user_id_type uid) noexcept {
            m_uid = uid;
            return *this;
        }

        /**
         * Set user id of this object.
         * Sets uid to 0 (anonymous) if the given uid is smaller than 0.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_uid_from_signed(signed_user_id_type uid) noexcept {
            m_uid = uid < 0 ? 0 : static_cast<user_id_type>(uid);
            return *this;
        }

        /**
         * Set user id of this object.
         *
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_uid(const char* uid) {
            m_uid = string_to_uid(uid);
            return *this;
        }

        /// Is this user anonymous?
        bool user_is_anonymous() const noexcept {
            return m_uid == 0;
        }

        /// Get timestamp when this object last changed.
        osmium::Timestamp timestamp() const noexcept {
            return m_timestamp;
        }

        /**
         * Set the timestamp when this object last changed.
         *
         * @param timestamp Timestamp
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_timestamp(const osmium::Timestamp& timestamp) noexcept {
            m_timestamp = timestamp;
            return *this;
        }

        /**
         * Set the timestamp when this object last changed.
         *
         * @param timestamp Timestamp in ISO format.
         * @returns Reference to object to make calls chainable.
         * @throws std::invalid_argment if the timestamp isn't a correctly ISO
         *                              formatted string with the Z timezone.
         *
         * @pre @code timestamp != nullptr @endcode
         */
        OSMObject& set_timestamp(const char* timestamp) {
            assert(timestamp);
            m_timestamp = detail::parse_timestamp(timestamp);
            if (timestamp[20] != '\0') {
                throw std::invalid_argument{"can not parse timestamp: garbage after timestamp"};
            }
            return *this;
        }

        /// Get user name for this object.
        const char* user() const noexcept {
            return reinterpret_cast<const char*>(data() + sizeof_object());
        }

        /// Clear user name.
        void clear_user() noexcept {
            std::memset(data() + sizeof_object(), 0, user_size());
        }

        /// Get the list of tags for this object.
        const TagList& tags() const {
            return osmium::detail::subitem_of_type<const TagList>(cbegin(), cend());
        }

        /**
         * Get tag value by key.
         *
         * Convenience function that will forward to same function on TagList
         * object.
         */
        const char* get_value_by_key(const char* key, const char* default_value = nullptr) const noexcept {
            return tags().get_value_by_key(key, default_value);
        }

        /**
         * Set named attribute.
         *
         * @param attr Name of the attribute (must be one of "id", "version", "changeset", "timestamp", "uid", "visible")
         * @param value Value of the attribute
         * @returns Reference to object to make calls chainable.
         */
        OSMObject& set_attribute(const char* attr, const char* value) {
            if (!std::strcmp(attr, "id")) {
                set_id(value);
            } else if (!std::strcmp(attr, "version")) {
                set_version(value);
            } else if (!std::strcmp(attr, "changeset")) {
                set_changeset(value);
            } else if (!std::strcmp(attr, "timestamp")) {
                set_timestamp(value);
            } else if (!std::strcmp(attr, "uid")) {
                set_uid(value);
            } else if (!std::strcmp(attr, "visible")) {
                set_visible(value);
            }

            return *this;
        }

        using iterator       = osmium::memory::CollectionIterator<Item>;
        using const_iterator = osmium::memory::CollectionIterator<const Item>;

        /**
         * Remove all tags from this object.
         *
         * (This will not change the size of the object, the tags are simply
         * marked as removed.)
         */
        void remove_tags() noexcept {
            for (auto& subitem : *this) {
                if (subitem.type() == osmium::item_type::tag_list) {
                    subitem.set_removed(true);
                }
            }
        }

        iterator begin() {
            return iterator(subitems_position());
        }

        iterator end() {
            return iterator(next());
        }

        const_iterator cbegin() const {
            return const_iterator(subitems_position());
        }

        const_iterator cend() const {
            return const_iterator(next());
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }

        /**
         * Get a range of subitems of a specific type.
         *
         * @tparam The type (must be derived from osmium::memory::Item).
         */
        template <typename T>
        osmium::memory::ItemIteratorRange<T> subitems() {
            static_assert(std::is_base_of<osmium::memory::Item, T>::value, "T must be derived from osmium::memory::Item");
            return osmium::memory::ItemIteratorRange<T>{subitems_position(), next()};
        }

        /**
         * Get a range of subitems of a specific type.
         *
         * @tparam The type (must be derived from osmium::memory::Item).
         */
        template <typename T>
        osmium::memory::ItemIteratorRange<const T> subitems() const {
            static_assert(std::is_base_of<osmium::memory::Item, T>::value, "T must be derived from osmium::memory::Item");
            return osmium::memory::ItemIteratorRange<const T>{subitems_position(), next()};
        }

        template <typename T>
        using t_iterator = osmium::memory::ItemIterator<T>;

        template <typename T>
        using t_const_iterator = osmium::memory::ItemIterator<const T>;

        template <typename T>
        t_iterator<T> begin() {
            return t_iterator<T>(subitems_position(), next());
        }

        template <typename T>
        t_iterator<T> end() {
            return t_iterator<T>(next(), next());
        }

        template <typename T>
        t_const_iterator<T> cbegin() const {
            return t_const_iterator<T>(subitems_position(), next());
        }

        template <typename T>
        t_const_iterator<T> cend() const {
            return t_const_iterator<T>(next(), next());
        }

        template <typename T>
        t_const_iterator<T> begin() const {
            return cbegin<T>();
        }

        template <typename T>
        t_const_iterator<T> end() const {
            return cend<T>();
        }

    }; // class OSMObject

    static_assert(sizeof(OSMObject) % osmium::memory::align_bytes == 0, "Class osmium::OSMObject has wrong size to be aligned properly!");

    /**
     * OSMObjects are equal if their type, id, and version are equal.
     */
    inline bool operator==(const OSMObject& lhs, const OSMObject& rhs) noexcept {
        return lhs.type()    == rhs.type() &&
               lhs.id()      == rhs.id()   &&
               lhs.version() == rhs.version();
    }

    inline bool operator!=(const OSMObject& lhs, const OSMObject& rhs) noexcept {
        return !(lhs == rhs);
    }

    /**
     * OSMObjects can be ordered by type, id, version, and timestamp. Usually
     * ordering by timestamp is not necessary as there shouldn't be two
     * objects with the same type, id, and version. But this can happen when
     * creating diff files from extracts, so we take the timestamp into
     * account here.
     *
     * Note that we use the absolute value of the id for a better ordering
     * of objects with negative id. All the negative IDs come first, then the
     * positive IDs. IDs are ordered by their absolute values. (This is the
     * same ordering JOSM uses.)
     *
     * See object_order_type_id_reverse_version if you need a different
     * ordering.
     */
    inline bool operator<(const OSMObject& lhs, const OSMObject& rhs) noexcept {
        return const_tie(lhs.type(), lhs.id() > 0, lhs.positive_id(), lhs.version(),
                   ((lhs.timestamp().valid() && rhs.timestamp().valid()) ? lhs.timestamp() : osmium::Timestamp())) <
               const_tie(rhs.type(), rhs.id() > 0, rhs.positive_id(), rhs.version(),
                   ((lhs.timestamp().valid() && rhs.timestamp().valid()) ? rhs.timestamp() : osmium::Timestamp()));
    }

    inline bool operator>(const OSMObject& lhs, const OSMObject& rhs) noexcept {
        return rhs < lhs;
    }

    inline bool operator<=(const OSMObject& lhs, const OSMObject& rhs) noexcept {
        return !(rhs < lhs);
    }

    inline bool operator>=(const OSMObject& lhs, const OSMObject& rhs) noexcept {
        return !(lhs < rhs);
    }

} // namespace osmium

#endif // OSMIUM_OSM_OBJECT_HPP
