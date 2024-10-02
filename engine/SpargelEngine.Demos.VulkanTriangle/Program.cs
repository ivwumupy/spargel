using System.Runtime.InteropServices;

namespace SpargelEngine.Demos.VulkanTriangle
{
  public class Program
  {
    public static void Main(string[] args)
    {
      var module = Binding.LoadLibraryA("vulkan-1.dll");
      unsafe
      {
        var vkGetInstanceProcAddr =
          (delegate* unmanaged[Stdcall]<IntPtr, IntPtr, IntPtr>)Binding.GetProcAddress(module, "vkGetInstanceProcAddr");
        var vkCreateInstance =
          (delegate* unmanaged[Stdcall]<VkInstanceCreateInfo*, IntPtr, IntPtr*, Int32>)
            vkGetInstanceProcAddr(0, Marshal.StringToHGlobalAnsi("vkCreateInstance"));
        var appInfo = new VkApplicationInfo
        {
          sType = 0,
          pNext = IntPtr.Zero,
          pApplicationName = Marshal.StringToHGlobalAnsi("VulkanTriangle"),
          applicationVersion = 0,
          pEngineName = Marshal.StringToHGlobalAnsi("SpargelEngine"),
          engineVersion = 0,
          apiVersion = (1 << 22) | (1 << 12),
        };
        var createInfo = new VkInstanceCreateInfo
        {
          sType = 1,
          pNext = IntPtr.Zero,
          flags = 0,
          pApplicationInfo = &appInfo,
          enabledLayerCount = 0,
          ppEnabledLayerNames = IntPtr.Zero,
          enabledExtensionCount = 0,
          ppEnabledExtensionNames = IntPtr.Zero,
        };
        IntPtr instance;
        var result = vkCreateInstance(&createInfo, IntPtr.Zero, &instance);
        if (result != 0)
        {
          Console.WriteLine("failed to create vulkan instance");
          return;
        }
        Console.WriteLine("instance created");

        var vkEnumeratePhysicalDevices =
          (delegate* unmanaged[Stdcall]<IntPtr, UInt32*, IntPtr*, Int32>)
            vkGetInstanceProcAddr(instance, Marshal.StringToHGlobalAnsi("vkEnumeratePhysicalDevices"));
        UInt32 count = 0;
        vkEnumeratePhysicalDevices(instance, &count, null);
        Console.WriteLine($"# of adapters = {count}");

        var vkDestroyInstance =
          (delegate* unmanaged[Stdcall]<IntPtr, IntPtr, void>)
            vkGetInstanceProcAddr(instance, Marshal.StringToHGlobalAnsi("vkDestroyInstance"));

        vkDestroyInstance(instance,  IntPtr.Zero);
      }
    }
  }

  internal struct VkInstanceCreateInfo
  {
    public Int32 sType;
    public IntPtr pNext;
    public Int32 flags;
    public unsafe VkApplicationInfo* pApplicationInfo;
    public UInt32 enabledLayerCount;
    public IntPtr ppEnabledLayerNames;
    public UInt32 enabledExtensionCount;
    public IntPtr ppEnabledExtensionNames;
  }

  internal struct VkApplicationInfo
  {
    public Int32 sType;
    public IntPtr pNext;
    public IntPtr pApplicationName;
    public UInt32 applicationVersion;
    public IntPtr pEngineName;
    public UInt32 engineVersion;
    public UInt32 apiVersion;
  }

  internal class Binding
  {
    [DllImport("kernel32.dll")]
    public static extern IntPtr LoadLibraryA(String file);
    [DllImport("kernel32.dll")]
    public unsafe static extern IntPtr GetProcAddress(IntPtr module, String name);
  }
}