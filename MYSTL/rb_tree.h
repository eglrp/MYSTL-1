#ifndef RB_TREE_H_
#define RB_TREE_H_

#include "allocator.h"
#include "construct.h"
#include "iterator.h"
#include "pair.h"

namespace mySTL {
    typedef bool rb_tree_color_type;
    const rb_tree_color_type rb_tree_red = false;
    const rb_tree_color_type rb_tree_black = true;

    //�����ü̳�
    template <class T>
    struct rb_tree_node {
        typedef rb_tree_color_type  color_type;
        typedef rb_tree_node*       link_type;
        
        color_type      color;
        link_type       parent;
        link_type       left;
        link_type       right;
        T               value;

        static link_type minimum(link_type* p) {
            while (p->left != nullptr)
                p = p->left;
            return p;
        }

        static link_type maximum(link_type* p) {
            while (p->right != nullptr)
                p = p->right;
            return p;
        }
    };

    template <class T>
    class rb_tree_iterator {
        typedef T                           value_type;
        typedef T&                          reference;
        typedef T*                          pointer;
        typedef rb_tree_iterator<T>         self;
        typedef rb_tree_iterator<T>         iterator;
        typedef rb_tree_node<T>*            link_type;
    public:     //����
        rb_tree_iterator() {}
        rb_tree_iterator(link_type x) { node = x; }
        rb_tree_iterator(const iterator& it) { node = it.node; }

    public:     //����
        reference operator* () const { return node->value; }
        pointer operator-> () const { return &(operator*()); }

        //ǰ��++
        self& operator++() {
            if (node->right != nullptr) {
                //�������������ӽڵ�
                node = node->right;
                while (node->left != nullptr)
                    node = node->left;
            }
            else {
                //û�����ӽڵ㣬��һֱ���ݣ�ֱ����ǰ�ڵ������ӽڵ�
                link_type p = node->parent;
                while (p->right == node) {
                    node = p;
                    p = p->parent;
                }
                if (node->right != p)
                    node = p;       //��ʱΪend
            }
            return *this;
        }

        //����++
        self operator++ (int) {
            self temp = *this;
            ++temp;
            return temp;
        }

        //ǰ��--
        self& operator-- () {
            //ͷ���
            if (node->color == rb_tree_red && node == node->parent->parent)
                node = node->right;
            else if (node->left != nullptr) {
                //����������
                link_type t = node->left;
                while (t->right != nullptr)
                    t = t->right;
                node = t;
            }
            else {
                //���ݣ�ֱ����ǰΪ���ӽڵ�
                link_type p = node->parent;
                while (p->left == node) {
                    p = node;
                    p = p->parent;
                }
                node = p;
            }
            return *this;
        }

        //����--
        self operator-- (int) {
            self t = *this;
            --t;
            return t;
        }

    public:
        link_type        node;
    };

    template <class T>
    inline bool operator== (const rb_tree_iterator<T>& x, const rb_tree_iterator<T>& y) {
        return x.node == y.node;
    }

    template <class T>
    inline bool operator!= (const rb_tree_iterator<T>& x, const rb_tree_iterator<T>& y) {
        return x.node != y.node;
    }

    //�����
    template <class Key, class Value, class KeyOfValue, class Compare,
    class Alloc = allocator<rb_tree_node<Value>>>
    class rb_tree {
    protected:
        typedef void*                       void_pointer;
        typedef Alloc                       rb_tree_node_allocator;
        typedef rb_tree_node<Value>         tree_node;
        typedef rb_tree_color_type          color_type;

    public:
        typedef Key                         key_type;
        typedef Value                       value_type;
        typedef value_type*                 pointer;
        typedef value_type&                 reference;
        typedef tree_node*                  link_type;
        typedef size_t                      size_type;
        typedef ptrdiff_t                   difference_type;
        typedef rb_tree_iterator<Value>     iterator;

    protected:
        link_type get_node() {
            return rb_tree_node_allocator::allocate();
        }

        void delete_node(link_type p) {
            rb_tree_node_allocator::deallocate(p);
        }

        link_type creat_node(const value_type& x) {
            link_type temp = get_node();
            construct(&temp->value, x);
            return temp;
        }

        //���ƽڵ�ֵ����ɫ
        link_type clone_node(link_type x) {
            link_type temp = creat_node(x->value);
            temp->color = x->color;
            temp->left = nullptr;
            temp->right = nullptr;
            temp->parent = nullptr;
            return temp;
        }

        void destroy_node(link_type x) {
            destroy(&x->value);
            delete_node(x);
        }

    protected:
        size_type   node_count;
        link_type   header;         //ͷָ��
        Compare     key_compare;

        link_type& root() const { return header->parent; }
        link_type& leftmost() const { return header->left };
        link_type& rightmost() const { return header->right; }

        //ȡ�ڵ��Ա
        static link_type& left(link_type x) { return x->left; }
        static link_type& right(link_type x) { return x->right; }
        static link_type& parent(link_type x) { return x->parent; }
        static reference value(link_type x) { return x->value; }
        static color_type& color(link_type x) { return x->color; }
        static const Key& key(link_type x) { return KeyOfValue()(value(x)); }

        //��ֵ
        static link_type& minimum(link_type x) {
            return rb_tree_node::minimum(x);
        }

        static link_type& maximum(link_type x) {
            return rb_tree_node::maximum(x);
        }

    public:     //��������
        rb_tree(const Compare& comp = Compare())
            : node_count(0), key_compare(comp) {
            init();
        }
        //������ֵ
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& operator= (const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);

        ~rb_tree() {
            clear();
            delete_node(header);
        }

    private:
        iterator __insert(link_type x, link_type y, const value_type& x);
        link_type __copy(link_type x, link_type p);
        iterator __erase(link_type x);
        void init() {
            header = get_node();
            color(header) = rb_tree_red;    //headerΪ��
            root() = nullptr;
            leftmost() = header;            //���ҽڵ�ָ���Լ�
            rightmost() = header;
        }

    public:
        Compare key_comp() const { return key_comp; }
        iterator begin() { return leftmost(); }
        iterator end() { return header; }
        bool empty() const { return node_count == 0; }
        size_type size() const { return node_count; }

    public:
        pair<iterator, bool> insert_unique(const value_type& x);
        iterator insert_equal(const value_type& x);
    };


}



#endif