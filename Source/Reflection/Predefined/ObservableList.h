/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_TYPES_OBSERVABLELIST
#define VCZH_REFLECTION_TYPES_OBSERVABLELIST
 
#include <Vlpp.h>
 
namespace vl
{
	namespace collections
	{
		/// <summary>Base type of observable container which triggers callbacks whenever items are changed.</summary>
		/// <typeparam name="T">Type of elements.</typeparam>
		/// <remarks>
		/// <p>Methods are the same to <see cref="List`2"/>, except that operator[] is readonly.</p>
		/// <p>
		/// When an item is being inserted to the list,
		/// <b>QueryInsert</b> will be called to determine if this item can be inserted,
		/// <b>BeforeInsert</b> will be called before inserting,
		/// <b>AfterInsert</b> will be called after inserting.
		/// </p>
		/// <p>
		/// When an item is being removed from the list,
		/// <b>QueryRemove</b> will be called to determine if this item can be removed,
		/// <b>BeforeRemove</b> will be called before removing,
		/// <b>AfterRemove</b> will be called after removing.
		/// </p>
		/// <p>
		/// When an item is being replaced, it is considered as removing the original item and inserting the new item.
		/// </p>
		/// <p>
		/// After any changing happens, <b>NotifyUpdateInternal</b> is called.
		/// Arguments is exactly the same as <see cref="reflection::description::IValueObservableList::ItemChanged"/>.
		/// </p>
		/// </remarks>
		template<typename T>
		class ObservableListBase : public collections::EnumerableBase<T>
		{
			using K = typename KeyType<T>::Type;
		protected:
			collections::List<T>					items;

			virtual void NotifyUpdateInternal(vint start, vint count, vint newCount)
			{
			}

			virtual bool QueryInsert(vint index, const T& value)
			{
				return true;
			}

			virtual void BeforeInsert(vint index, const T& value)
			{
			}

			virtual void AfterInsert(vint index, const T& value)
			{
			}

			virtual bool QueryRemove(vint index, const T& value)
			{
				return true;
			}

			virtual void BeforeRemove(vint index, const T& value)
			{
			}

			virtual void AfterRemove(vint index, vint count)
			{
			}

		public:
			collections::IEnumerator<T>* CreateEnumerator()const
			{
				return items.CreateEnumerator();
			}

			/// <summary>Trigger <b>NotifyUpdateInternal</b> manually.</summary>
			/// <returns>Returns true if arguments are not out of range.</returns>
			/// <param name="start">The index of the first item that are changed.</param>
			/// <param name="count">The number of items that are changed, the default value is 1.</param>
			/// <remarks>
			/// <p>
			/// This is useful when the container is not actually changed, but data in some items are changed.
			/// For example, in an observable list of shared pointers,
			/// properties of elements are changed does not trigger callbacks because it doesn't change pointers in the list.
			/// </p>
			/// <p>
			/// If subscribers need to know about such change, calling this function is an easy way to do it.
			/// </p>
			/// </remarks>
			bool NotifyUpdate(vint start, vint count = 1)
			{
				if (start<0 || start >= items.Count() || count <= 0 || start + count>items.Count())
				{
					return false;
				}
				else
				{
					NotifyUpdateInternal(start, count, count);
					return true;
				}
			}

			bool Contains(const K& item)const
			{
				return items.Contains(item);
			}

			vint Count()const
			{
				return items.Count();
			}

			vint Count()
			{
				return items.Count();
			}

			const T& Get(vint index)const
			{
				return items.Get(index);
			}

			const T& operator[](vint index)const
			{
				return items.Get(index);
			}

			vint IndexOf(const K& item)const
			{
				return items.IndexOf(item);
			}

			vint Add(const T& item)
			{
				return Insert(items.Count(), item);
			}

			bool Remove(const K& item)
			{
				vint index = items.IndexOf(item);
				if (index == -1) return false;
				return RemoveAt(index);
			}

			bool RemoveAt(vint index)
			{
				if (0 <= index && index < items.Count() && QueryRemove(index, items[index]))
				{
					BeforeRemove(index, items[index]);
					T item = items[index];
					items.RemoveAt(index);
					AfterRemove(index, 1);
					NotifyUpdateInternal(index, 1, 0);
					return true;
				}
				return false;
			}

			bool RemoveRange(vint index, vint count)
			{
				if (count <= 0) return false;
				if (0 <= index && index<items.Count() && index + count <= items.Count())
				{
					for (vint i = 0; i < count; i++)
					{
						if (!QueryRemove(index + 1, items[index + i])) return false;
					}
					for (vint i = 0; i < count; i++)
					{
						BeforeRemove(index + i, items[index + i]);
					}
					items.RemoveRange(index, count);
					AfterRemove(index, count);
					NotifyUpdateInternal(index, count, 0);
					return true;
				}
				return false;
			}

			bool Clear()
			{
				vint count = items.Count();
				for (vint i = 0; i < count; i++)
				{
					if (!QueryRemove(i, items[i])) return false;
				}
				for (vint i = 0; i < count; i++)
				{
					BeforeRemove(i, items[i]);
				}
				items.Clear();
				AfterRemove(0, count);
				NotifyUpdateInternal(0, count, 0);
				return true;
			}

			vint Insert(vint index, const T& item)
			{
				if (0 <= index && index <= items.Count() && QueryInsert(index, item))
				{
					BeforeInsert(index, item);
					items.Insert(index, item);
					AfterInsert(index, item);
					NotifyUpdateInternal(index, 0, 1);
					return index;
				}
				else
				{
					return -1;
				}
			}

			bool Set(vint index, const T& item)
			{
				if (0 <= index && index < items.Count())
				{
					if (QueryRemove(index, items[index]) && QueryInsert(index, item))
					{
						BeforeRemove(index, items[index]);
						items.RemoveAt(index);
						AfterRemove(index, 1);

						BeforeInsert(index, item);
						items.Insert(index, item);
						AfterInsert(index, item);

						NotifyUpdateInternal(index, 1, 1);
						return true;
					}
				}
				return false;
			}
		};

		/// <summary>An observable container that maintain an implementation of <see cref="reflection::description::IValueObservableList"/>.</summary>
		/// <typeparam name="T">Type of elements.</typeparam>
		template<typename T>
		class ObservableList : public ObservableListBase<T>
		{
		protected:
			void NotifyUpdateInternal(vint start, vint count, vint newCount) override;

		public:
		};

		namespace randomaccess_internal
		{
			template<typename T>
			struct RandomAccessable<ObservableListBase<T>>
			{
				static const bool							CanRead = true;
				static const bool							CanResize = false;
			};

			template<typename T>
			struct RandomAccessable<ObservableList<T>>
			{
				static const bool							CanRead = true;
				static const bool							CanResize = false;
			};
		}
	}
}

#endif